#include <exception.h>
#include <csr.h>

Exception::Exception() {
    __csrwr_w((1 << 13) - 1, CSR_ECFG);
}

void Exception::IntOff() {
    u32 crmd = __csrrd_w(CSR_CRMD);
    crmd &= (~(1u << 2));
    __csrwr_w(crmd, CSR_CRMD);
}

void Exception::IntOn(){
    u32 crmd = __csrrd_w(CSR_CRMD);
    crmd |= (1u << 2);
    __csrwr_w(crmd, CSR_CRMD);
}

void Exception::HandleDefaultException() {
    u64 estate = __csrrd_d(CSR_ESTAT);
    switch (getPartical(estate, 21, 16)) {
        case 0: {
            for (u8 intrOp = 12; intrOp >= 0; --intrOp) {
                if (estate & (1 << intrOp)) {
                    switch (intrOp) {
                        case 11:
                            ++Jiffies::sysJiffies;
                            processController.HandleSchedule();
                            SysTimer.TimerIntClear();
                    }
                    break;
                }
            }
            break;
        }
        case 0xB: {
            extern u64 ContextReg[30];
            char* addr;
            switch (ContextReg[9]) {
                //chdir
                case 49:
                    addr = (char*)processController.CurrentProcess->GetSpace()->MMUService.V2P(ContextReg[2]);
                    ContextReg[2] = processController.CurrentProcess->SdFileTable.Chdir((u8*)addr);
                    break;
                //openat
                case 56:
                    addr = (char*)processController.CurrentProcess->GetSpace()->MMUService.V2P(ContextReg[3]);
                    ContextReg[2] = processController.CurrentProcess->SdFileTable.Open(addr);
                    break;
                //close
                case 57:
                    processController.CurrentProcess->SdFileTable.Close(ContextReg[2]);
                    break;
                //read
                case 63:
                    if (ContextReg[2] != 0) {
                        addr = (char*)processController.CurrentProcess->GetSpace()->MMUService.V2P(ContextReg[3]);
                        ContextReg[2] = processController.CurrentProcess->SdFileTable.Read(ContextReg[2], (u8*) addr, ContextReg[4]);
                    }
                    break;
                //write
                case 64:
                    addr = (char*)processController.CurrentProcess->GetSpace()->MMUService.V2P(ContextReg[3]);
                    if (ContextReg[2] == 1 || ContextReg[2] == 2) {
                        for (u64 i = 0; i < ContextReg[4]; ++i) {
                            uPut << *(addr + i);
                        }
                        ContextReg[2] = ContextReg[4];
                    } else {
                        ContextReg[2] = processController.CurrentProcess->SdFileTable.Read(ContextReg[2], (u8*) addr, ContextReg[4]);
                    }
                    break;
                //getcwd
                case 17:
                    addr = (char*)processController.CurrentProcess->GetSpace()->MMUService.V2P(ContextReg[2]);
                    ContextReg[2] = processController.CurrentProcess->SdFileTable.Getcwd((u8*)addr,ContextReg[3]);
                    break;
                //fork
                // case 65:
                //     addr = (char )
                //     break;
                //exit
                case 93:
                    delete processController.CurrentProcess;
                    processController.CurrentProcess = nullptr;
                    processController.HandleSchedule();
                    break;
                default:
                    uPut << "Unsupported Syscall " << ContextReg[9] << "\r\n";
                    uPut << "ERA: " << (void*) __csrrd_d(CSR_ERA) << "\r\n";
                    while (1);
                    break;
            }
            __csrwr_d(__csrrd_d(CSR_ERA) + 4, CSR_ERA);
            break;
        }
        case 0x1:
        case 0x2:
        case 0x3:
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7: {
            uPut << "**Page Invalid**\r\n";
            uPut << "ESTATE: " << (void*) estate << "\r\n";
            uPut << "ERA: " << (void*) __csrrd_d(CSR_ERA) << "\r\n";
            uPut << "BADV: " << (void*) __csrrd_d(CSR_BADV) << "\r\n";
            uPut << "TLBEHI: " << (void*) __csrrd_d(CSR_TLBEHI) << "\r\n";
            __asm__(
                "tlbsrch\n"
                "tlbrd"
            );
            uPut << "TLBIDX: " << (void*) __csrrd_d(CSR_TLBIDX) << "\r\n";
            uPut << "TLBELO0: " << (void*) __csrrd_d(CSR_TLBELO0) << "\r\n";
            uPut << "TLBELO1: " << (void*) __csrrd_d(CSR_TLBELO1) << "\r\n";
            while (1);
        }
        default: {
            uPut << "**Exception**\r\n";
            uPut << "ESTATE: " << (void*) estate << "\r\n";
            uPut << "ERA: " << (void*) __csrrd_d(CSR_ERA) << "\r\n";
            uPut << "BADV: " << (void*) __csrrd_d(CSR_BADV) << "\r\n";
            uPut << "BADI: " << (void*) __csrrd_d(CSR_BADI) << "\r\n";
            while (1);
        }
    }
}

void Exception::HandleTLBException() {
    if (!processController.CurrentProcess) {
        uPut << "**Kernel Memory Error**\r\n";
        uPut << "TLBRBADV: " << (void*) __csrrd_d(CSR_TLBRBADV) << "\r\n";
        uPut << "TLBRERA: " << (void*) __csrrd_d(CSR_TLBRERA) << "\r\n";
        while (1);
    }

    u64 addr = __csrrd_d(CSR_TLBRBADV);
    auto& mmu = processController.CurrentProcess->GetSpace()->MMUService;

    if (mmu.TryLoadTLB(addr)) {
        return;
    }

    auto zone = processController.CurrentProcess->GetSpace()->ZoneTree->find([addr](Zone* t)->u8{
        if (t->VStart <= addr && addr <= t->VEnd) return 1;
        else if (addr < t->VStart) return 0;
        else return 2;
    });
    if (zone == nullptr) {
        uPut << "**illegal address**\r\n";
        uPut << "TLBRBADV: " << (void*) addr << "\r\n";
        uPut << "TLBRERA: " << (void*) __csrrd_d(CSR_TLBRERA) << "\r\n";
        while (1);
    }
    zone->val->OnPageFault(addr);
}

void Exception::HandleMachineError(){
    uPut << "**Machine Error**\r\n";
    uPut << "MERRERA: " << (void*) __csrrd_d(CSR_MERRERA) << "\r\n";
    uPut << "MERRCTL: " << (void*) __csrrd_d(CSR_MERRCTL) << "\r\n";
    uPut << "MERRINFO1: " << (void*) __csrrd_d(CSR_MERRINFO1) << "\r\n";
    uPut << "MERRINFO2: " << (void*) __csrrd_d(CSR_MERRINFO2) << "\r\n";
    while (1);
}
