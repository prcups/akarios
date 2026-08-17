#include <uart.h>
#include <mem.h>
#include <util.h>
#include <acpi.h>
#include <pcie.h>
#include <exception.h>
#include <larchintrin.h>
#include <timer.h>
#include <process.h>
#include <disk.h>
#include <list.h>
#include <sdcard.h>
#include <fat32.h>
#include <gpt.h>
#include "include/csr.h"

UART uPut((u8 *)(0x800000001fe001e0llu));
Exception SysException;
Timer SysTimer;

PageAllocator pageAllocator;
SmallMemAllocator smallMemAllocator;
ProcessController processController;
ACPIManager acpiManager;
PCIEDeviceManager pcieDeviceManager;

ListItem<Disk*> *diskList = nullptr;
ListItem<FileSystem*> *fsList = nullptr;

extern "C" void StartProcess();

extern "C" {
    void __cxa_pure_virtual() {}

    void handleDefaultException() {
        SysException.HandleDefaultException();
    }

    void handleTLBException() {
        SysException.HandleTLBException();
    }

    void handleMachineError() {
        SysException.HandleMachineError();
    }
}

inline void invokeInit() {
    using func_ptr = void (*) (void);
    extern char __init_array_start, __init_array_end;
    for (func_ptr *func = (func_ptr *) &__init_array_start; func != (func_ptr *) &__init_array_end; ++func) {
        if (func) (*func)();
    }
}

inline void initMem() {
    __csrwr_d(0x13E4D52C, CSR_PWCL);
    __csrwr_d(0x267, CSR_PWCH);
    __csrwr_d(0xC, CSR_STLBPS);
}

inline void initException() {
    extern void *HandleDefaultExceptionEntry, *HandleMachineErrorEntry, *HandleTLBExceptionEntry;
    __csrwr_d((u64)&HandleDefaultExceptionEntry, CSR_EENTRY);
    __csrwr_d((u64)&HandleTLBExceptionEntry, CSR_TLBRENTRY);
    __csrwr_d((u64)&HandleMachineErrorEntry, CSR_MERRENTRY);

    SysException.IntOn();
}

void printTestFile() {
    if (fsList == nullptr) {
        diskList == nullptr ? uPut << "NO DISK\n" : uPut << "NO FS\n";
        return;
    }

    FileSystem *fs = fsList->Val;
    fs->Mount();

    FileHandle file;
    KernelUtil::Memset(&file, 0, sizeof(file));
    if (fs->Open("/test.txt", &file) != (int)FsError::Ok) {
        uPut << "Failed to open /test.txt\n";
        return;
    }

    uPut << "/test.txt:\n";
    u8 buf[512];
    int bytesRead;
    while ((bytesRead = fs->Read(&file, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < bytesRead; ++i) {
            uPut << (char)buf[i];
        }
    }
    uPut << '\n';
}

extern "C" void KernelMain(BootInfo info) {
    invokeInit();
    pageAllocator.Init(info);
    initMem();
    initException();
    acpiManager.Init(info.XsdpPtr);
    pcieDeviceManager.Init();

    for (ListItem<Disk*> *item = diskList; item != nullptr; item = item->Next) {
        ScanDiskFileSystems(item->Val, &fsList);
    }
    printTestFile();

    //SysTimer.TimerOn();
    //StartProcess();

    while (1);
}
