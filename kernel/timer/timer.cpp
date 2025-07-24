#include <timer.h>

Timer::Timer() {
    __csrwr_d(0x800002, 0x41);
}

void Timer::TimerOff(){
    u64 tcfg = __csrrd_d(0x41);
    tcfg &= (~1ull);
    __csrwr_d(tcfg, 0x41);
}

void Timer::TimerOn() {
    u64 tcfg = __csrrd_d(0x41);
    tcfg |= 1;
    __csrwr_d(tcfg, 0x41);
}

void Timer::TimerIntClear() {
    __csrwr_d(1, 0x44);
}

