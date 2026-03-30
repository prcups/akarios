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

alignas(8192) void proc1_exec() {
    char ch = 'A';
	char *s = &ch;
loop1:
    __asm__ (
        "li.d $a0, 1\n\t"
        "ld.d $a1, %0\n\t"
        "li.d $a2, 1\n\t"
        "li.d $a7, 64\n\t"
        "syscall 0\n\t"
        ::"m"(s)
        :"memory", "$a0", "$a1", "$a2", "$a7"
    );
    for (int i = 0; i < 10000; ++i);
    goto loop1;
}

alignas(8192) void proc2_exec() {
    char ch = 'B';
    char *s = &ch;
loop2:
    __asm__ (
        "li.d $a0, 1\n\t"
        "ld.d $a1, %0\n\t"
        "li.d $a2, 1\n\t"
        "li.d $a7, 64\n\t"
        "syscall 0\n\t"
        ::"m"(s)
        :"memory", "$a0", "$a1", "$a2", "$a7"
    );
    for (int i = 0; i < 10000; ++i);
    goto loop2;
}

extern "C" void KernelMain(BootInfo info) {
    invokeInit();
    pageAllocator.Init(info);
    initMem();
    initException();
    acpiManager.Init(info.XsdpPtr);
    pcieDeviceManager.Init();

    Process *proc1, *proc2;
    proc1 = new Process(0, 0, 0x100000);
    proc2 = new Process(0, 0, 0x100000);
    processController.InsertProcess(proc1);
    proc1->GetSpace()->AddZone(new TNode<Zone>(new DirectZone(0x100000, 0x200000, (u64) proc1_exec, ZoneConfig{1, 3, 0, 0, 0, 0})));
    processController.InsertProcess(proc2);
    proc2->GetSpace()->AddZone(new TNode<Zone>(new DirectZone(0x100000, 0x200000, (u64) proc2_exec, ZoneConfig{1, 3, 0, 0, 0, 0})));

    SysTimer.TimerOn();
    StartProcess();

    while (1);
}
