#include <uart.h>
#include <mem.h>
#include <util.h>
#include <acpi.h>
#include <pcie.h>
#include <exception.h>
#include <larchintrin.h>
#include <timer.h>
#include <process.h>

UART uPut((u8 *)(0x800000001fe001e0llu));
Exception SysException;
Timer SysTimer;

PageAllocator pageAllocator;
SmallMemAllocator smallMemAllocator;
ProcessController processController;
ACPIManager acpiManager;
PCIEDeviceManager pcieDeviceManager;

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
    __csrwr_d(0x13E4D52C, 0x1C);
    __csrwr_d(0x267, 0x1D);
    __csrwr_d(0xC, 0x1E);
}

inline void initException() {
    extern void *HandleDefaultExceptionEntry, *HandleMachineErrorEntry, *HandleTLBExceptionEntry;
    __csrwr_d((u64)&HandleDefaultExceptionEntry, 0xC);
    __csrwr_d((u64)&HandleTLBExceptionEntry, 0x88);
    __csrwr_d((u64)&HandleMachineErrorEntry, 0x93);

    SysException.IntOn();
}

extern "C" void KernelMain(BootInfo info) {
    invokeInit();
    pageAllocator.Init(info);
    initMem();
    initException();
    acpiManager.Init(info.XsdpPtr);
    pcieDeviceManager.Init();

    //SysTimer.TimerOn();


    while (1);
}
