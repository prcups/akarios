#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <util.h>
#include <uart.h>
#include <timer.h>
#include <larchintrin.h>
#include <compare>
#include <process.h>

class Exception {

public:
    void HandleMachineError();
    void HandleTLBException();
    void HandleDefaultException();
    Exception();
    void IntOn();
    void IntOff();
};

extern Exception SysException;
#endif
