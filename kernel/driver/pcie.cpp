#include <pcie.h>

void PCIEDeviceManager::Init()
{
    ACPISDTHeader* mcfg = acpiManager.FindTable("MCFG");
    ecmSpace = (ECMSpace *) ((u64) mcfg + 44);
    ecmSpaceNum = (mcfg->Length - 44) / 16;
    pcieSegList = new PCIESeg[ecmSpaceNum];
    for (u8 i = 0; i < ecmSpaceNum; ++i)
        pcieSegList[i].Init(ecmSpace[i].BaseAddress);
}

void PCIESeg::Init(u64 baseAddress)
{
    this->baseAddress = baseAddress;
    visitBus(0);
}

PCIDeviceHeader * PCIESeg::getDevice(u8 busNum, u8 devNum, u8 funcNum)
{
    return (PCIDeviceHeader*) (baseAddress +
        ((busNum << 20) | (devNum << 15) | (funcNum << 12)));
}

void PCIESeg::visitBus(u8 busNum)
{
    for (u8 devNum = 0; devNum < 32; ++devNum)
        visitDevice(busNum, devNum);
}

void PCIESeg::visitDevice(u8 busNum, u8 devNum)
{
    auto *dev = getDevice(busNum, devNum, 0);
    if (dev->VendorID == 0xFFFF) return;
    load(dev);
    if (dev->MF)
    {
        PCIDeviceHeader *func;
        for (u8 funcNum = 1; funcNum < 8; ++funcNum)
        {
            func = getDevice(busNum, devNum, funcNum);
            if (func->VendorID == 0xFFFF) return;
            load(func);
        }
    }
}

void PCIESeg::load(PCIDeviceHeader* dev)
{
    if (dev->HeaderType == 0) loadDevice((PCIDevice*) dev);
    else if (dev->HeaderType == 1) loadBridge((PCIBridge*) dev);
}


void PCIESeg::loadDevice(PCIDevice* dev)
{
    uPut << dev->ClassCode << " " << dev->SubClass << '\n';
//     switch (dev->ClassCode)
//     {
//         case 0x1:
//
//     }
}

void PCIESeg::loadBridge(PCIBridge *dev)
{
    dev->PrimaryBusNumber = busPos;
    dev->SecondaryBusNumber = ++busPos;
    visitBus(dev->SecondaryBusNumber);
    dev->SubordinateBusNumber = busPos;
}
