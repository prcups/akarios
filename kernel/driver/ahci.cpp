#include <ahci.h>

#define NR_BUFFER 16
#define ADDRESS_UPPER (DMW_MASK >> 32)
#define ATA_CMD_READ_DMA_EX     0x25
#define ATA_CMD_WRITE_DMA_EX     0x35
#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

#define HBA_PxIS_TFES   (1 << 30)

struct buffer
{
	char *data;
	short blocknr;
};

AHCIController::AHCIController(u64 address):abar((HBA_MEM*) address)
{
	// Search disk in implemented ports
	u32 pi = abar->pi;
	int i = 0;
	while (i<32)
	{
		if (pi & 1)
		{
			int dt = check_type(&abar->ports[i]);
			if (dt == AHCI_DEV_SATA)
				ListItem<Disk*>::Add(&diskList, new ListItem<Disk*>(new SATADisk(abar, i)));
		}
		pi >>= 1;
		++i;
	}
}

int AHCIController::check_type(HBA_PORT *port)
{
	u32 ssts = port->ssts;

	u8 ipm = (ssts >> 8) & 0x0F;
	u8 det = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT)
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;

	switch (port->sig)
	{
		case SATA_SIG_ATAPI:
			return AHCI_DEV_SATAPI;
		case SATA_SIG_SEMB:
			return AHCI_DEV_SEMB;
		case SATA_SIG_PM:
			return AHCI_DEV_PM;
		default:
			return AHCI_DEV_SATA;
	}
}

SATADisk::SATADisk(HBA_MEM* abar, u8 portno)
:abar(abar), port(abar->ports + portno)
{
	cmdHeader = (HBA_CMD_HEADER *) pageAllocator.AllocPageMem(0);
	cmdTable = (HBA_CMD_TBL *) pageAllocator.AllocPageMem(0);
	port_rebase();
}

void SATADisk::start_cmd()
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR);

	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST;
}

void SATADisk::stop_cmd()
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;

	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;

	// Wait until FR (bit14), CR (bit15) are cleared
	while(1)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
}

void SATADisk::port_rebase()
{
	stop_cmd();	// Stop command engine

	abar->ghc |= 0x80000002;
	port->ie = 0xFFFFFFFF;

	cmdHeader = (HBA_CMD_HEADER*) pageAllocator.AllocPageMem(0);
	cmdTable = (HBA_CMD_TBL*) pageAllocator.AllocPageMem(1);
	KernelUtil::Memset((void*)cmdHeader, 0, 4096);
	KernelUtil::Memset((void*)cmdTable, 0, 8192);

	port->clb = (u64) cmdHeader;
	port->clbu = (u64) cmdHeader >> 32;

	// FIS offset: 32K+256*portno
	// FIS entry size = 256 bytes per port
	u64 fisAddr = (u64)cmdHeader + (1<<10);
	port->fb = fisAddr;
	port->fbu = fisAddr >> 32;
	fis = (FIS*) fisAddr;

	// Command table offset: 40K + 8K*portno
	// Command table size = 256*32 = 8K per port
	for (int i=0; i<32; i++)
	{
		cmdHeader[i].prdtl = 1;	// 8 prdt entries per command table
					// 256 bytes per command table, 64+16+48+16*8
		// Command table offset: 40K + 8K*portno + cmdheader_index*256
		cmdHeader[i].ctba = (u64) (cmdTable + i);
		cmdHeader[i].ctbau = (u64) (cmdTable + i) >> 32;
	}

	start_cmd();	// Start command engine
}

int SATADisk::find_cmdslot()
{
	// If not set in SACT and CI, the slot is free
	unsigned int slots = (port->sact | port->ci);
	for (int i=0; i<32; i++)
	{
		if ((slots&1) == 0)
			return i;
		slots >>= 1;
	}
	return -1;
}

void SATADisk::rw_disk(unsigned short blocknr, char *buf, int rw)
{
	port->is = 0xFFFFFFFF;		// Clear pending interrupt bits
	int slot = find_cmdslot();
	if (slot == -1)
		return;

	cmdHeader[slot].cfl = sizeof(FIS_REG_H2D)/sizeof(unsigned int);	// Command FIS size
	cmdHeader[slot].w = rw;		// Read or write from device
	cmdHeader[slot].prdtl = 1;	// PRDT entries count

	cmdTable[slot].prdt_entry[0].dba = (unsigned long) buf;
	cmdTable[slot].prdt_entry[0].dbau = (unsigned long) buf >> 32;
	cmdTable[slot].prdt_entry[0].dbc = (1<<9)-1;	// 512 bytes per sector
	cmdTable[slot].prdt_entry[0].i = 1;

	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*) cmdTable->cfis;

	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = rw == 0 ? ATA_CMD_READ_DMA_EX : ATA_CMD_WRITE_DMA_EX;

	cmdfis->lba0 = (unsigned char)blocknr;
	cmdfis->lba1 = (unsigned char)(blocknr>>8);
	cmdfis->lba2 = (unsigned char)(blocknr>>16);;
	cmdfis->device = 1<<6;	// LBA mode

	cmdfis->lba3 = (unsigned char)(blocknr>>24);
	cmdfis->lba4 = 0;
	cmdfis->lba5 = 0;

	cmdfis->countl = 1 & 0xFF;
	cmdfis->counth = (0 >> 8) & 0xFF;

	// The below loop waits until the port is no longer busy before issuing a new command
	while (port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ));
	port->ci = 1<<slot;	// Issue command

	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<slot)) == 0)
			break;
	}
}

void SATADisk::Read(u64 blockNum, char* buf)
{
	rw_disk(blockNum, buf, 0);
}

void SATADisk::Write(u64 blockNum, char* buf)
{
	rw_disk(blockNum, buf, 1);
}
