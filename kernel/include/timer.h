#ifndef TIMER_H
#define TIMER_H

#include <util.h>
#include <uart.h>
#include <larchintrin.h>

class Timer {
public:
    Timer();
    void TimerOn();
    void TimerOff();
    void TimerIntClear();
};

extern Timer SysTimer;

#endif
