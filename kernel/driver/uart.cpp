#include <uart.h>

UART::UART(u8 *addr) :baseAddress(addr){
    // disable interrupts
    *(baseAddress + 1) = 0;
    // special mode to set baud rate
    *(baseAddress + 3) = 0x80;
    // LSB for baud rate of 38.4K
    *baseAddress = 0x1B;
    // MSB for baud rate of 38.4K
    *(baseAddress + 1) = 0x0;
    // leave set-baud mode and set word length
    // to 8 bits, no parity
    *(baseAddress + 3) = 0x03;
    // reset and enable FIFOs
    *(baseAddress + 2) = 0x07;
    // enable transmit and receive interrupts.
    *(baseAddress + 1) = 0x03;
}

void UART::UARTPutchar(char c) {
    // wait for Transmit Holding Empty to be set in LSR
    while((*(baseAddress + 5) & (1 << 5)) == 0) {}
    *baseAddress = c;
}

UART & UART::operator << (const char* x)
{
    u32 n = KernelUtil::Strlen(x);
    for (int i = 0; i < n; ++i) {
        UARTPutchar(x[i]);
    }
    return *this;
}

UART & UART::operator << (char x)
{
    UARTPutchar(x);
    return *this;
}
