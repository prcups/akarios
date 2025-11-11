#ifndef PCIE_H
#define PCIE_H

#include <util.h>
#include <acpi.h>
#include <list.h>

struct ECMSpace
{
    u64 BaseAddress;
    u16 SectionGroupNumber;
    u8 StartBusNumber;
    u8 EndBusNumber;
    u32 resv;
};

struct PCIDeviceInfoHeader
{
    u16 VendorID;
    u16 DeviceID;
    u16 Command;
    u16 Status;
    u8 RevisionID;
    u8 ProgIF;
    u8 SubClass;
    u8 ClassCode;
    u8 CacheLineSize;
    u8 LatencyTimer;
    u8 MF:1;
    u8 HeaderType:7;
    u8 BIST;
};

struct PCIDeviceInfo : PCIDeviceInfoHeader
{
    u32 Bar[6];
    u32 CardbusCISPointer;
    u16 SubsystemVendorID;
    u16 SubsystemID;
    u32 ExpansionROMBaseAddress;
    u32 resv1:24;
    u32 CapabilityPointer:8;
    u32 resv2;
    u8 InterruptLine;
    u8 InterruptPin;
    u8 MinGrant;
    u8 MaxLatency;
};

struct PCIBridge : PCIDeviceInfoHeader
{
    u32 Bar[2];
    u8 PrimaryBusNumber;
    u8 SecondaryBusNumber;
    u8 SubordinateBusNumber;
    u8 SecondaryLatencyTimer;
    u8 IOBase;
    u8 IOLimit;
    u16 SecondaryStatus;
    u16 MemoryBase;
    u16 MemoryLimit;
    u16 PrefetchableMemoryBase;
    u16 PrefetchableMemoryLimit;
    u32 PrefetchableMemoryBaseUpper;
    u32 PrefetchableMemoryLimitUpper;
    u16 IOBaseUpper;
    u16 IOLimitUpper;
    u32 CapabilityPointer:8;
    u32 resv:24;
    u32 ExpansionROMBaseAddress;
    u8 InterruptLine;
    u8 InterruptPin;
    u16 BridgeControl;
};

class PCIEDevice {};

class PCIESeg
{
    u64 baseAddress;
    u32 busPos = 0;
    PCIDeviceInfoHeader* getDevice(u8 busNum, u8 devNum, u8 funcNum);
    void visitBus(u8 busNum);
    void visitDevice(u8 busNum, u8 devNum);
    void load(PCIDeviceInfoHeader *dev);
    void loadDevice(PCIDeviceInfo *dev);
    void loadBridge(PCIBridge *dev);
    ListItem<PCIEDevice*> *deviceList;
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
