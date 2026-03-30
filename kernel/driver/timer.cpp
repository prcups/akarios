#include <timer.h>
#include <csr.h>

Timer::Timer() {
    __csrwr_d(0x800002, CSR_TCFG);
}

void Timer::TimerOff(){
    u64 tcfg = __csrrd_d(CSR_TCFG);
    tcfg &= (~1ull);
    __csrwr_d(tcfg, CSR_TCFG);
}

void Timer::TimerOn() {
    u64 tcfg = __csrrd_d(CSR_TCFG);
    tcfg |= 1;
    __csrwr_d(tcfg, CSR_TCFG);
}

void Timer::TimerIntClear() {
    __csrwr_d(1, CSR_TICLR);
}

