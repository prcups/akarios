#ifndef PCIE_H
#define PCIE_H

#include <util.h>
#include <acpi.h>

struct ECMSpace
{
    u64 BaseAddress;
    u16 SectionGroupNumber;
    u8 StartBusNumber;
    u8 EndBusNumber;
    u32 resv;
};

struct PCIDeviceHeader
{
    u16 DeviceID;
    u16 VendorID;
    u16 Status;
    u16 Command;
    u8 ClassCode;
    u8 SubClass;
    u8 ProgIF;
    u8 RevisionID;
    u8 BIST;
    u8 MF:1;
    u8 HeaderType:7;
    u8 LatencyTimer;
    u8 CacheLineSize;
};

struct PCIDevice : PCIDeviceHeader
{
    u32 Bar[6];
    u32 CardbusCISPointer;
    u16 SubsystemID;
    u16 SubsystemVendorID;
    u32 ExpansionROMBaseAddress;
    u32 resv1:24;
    u32 CapabilityPointer:8;
    u32 resv2;
    u8 MaxLatency;
    u8 MinGrant;
    u8 InterruptPin;
    u8 InterruptLine;
};

struct PCIBridge : PCIDeviceHeader
{
    u32 Bar[2];
    u8 SecondaryLatencyTimer;
    u8 SubordinateBusNumber;
    u8 SecondaryBusNumber;
    u8 PrimaryBusNumber;
    u16 SecondaryStatus;
    u8 IOLimit;
    u8 IOBase;
    u16 MemoryLimit;
    u16 MemoryBase;
    u16 PrefetchableMemoryLimit;
    u16 PrefetchableMemoryBase;
    u32 PrefetchableMemoryLimitUpper;
    u32 PrefetchableMemoryBaseUpper;
    u16 IOLimitUpper;
    u16 IOBaseUpper;
    u32 resv:24;
    u32 CapabilityPointer:8;
    u32 ExpansionROMBaseAddress;
    u16 BridgeControl;
    u8 InterruptPin;
    u8 InterruptLine;
};

class PCIESeg
{
    u64 baseAddress;
    u32 busPos = 0;
    PCIDeviceHeader* getDevice(u8 busNum, u8 devNum, u8 funcNum);
    void visitBus(u8 busNum);
    void visitDevice(u8 busNum, u8 devNum);
    void load(PCIDeviceHeader *dev);
    void loadDevice(PCIDevice *dev);
    void loadBridge(PCIBridge *dev);
public:
    void Init(u64 baseAddress);
};

class PCIEDeviceManager
{
    ECMSpace *ecmSpace;
    u32 ecmSpaceNum;
    PCIESeg *pcieSegList;
public:
    void Init();
};

#endif
