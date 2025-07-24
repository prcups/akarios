#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

#include <util.h>

class UART {
    u8 *const baseAddress;
    void UARTPutchar(char c);
    template <typename T>
    void UARTPutint(T x, u16 s = 10)
    {
        if (x == 0) {
            return;
        }
        else {
            UARTPutint(x / s, s);
            u16 t = x % s;
            if (t >= 0 && t <= 9) {
                UARTPutchar(t + '0');
            } else {
                UARTPutchar(t - 10 + 'A');
            }
        }
    }
public:
    UART(u8 *addr);
    template<typename T>
    UART & operator << (T x)
    {
        if (x == 0) UARTPutchar('0');
        else UARTPutint(x);
        return *this;
    }

    template <typename T>
    UART & operator << (T* x)
    {
        UARTPutchar('0');
        UARTPutchar('x');
        if (x == 0) UARTPutchar('0');
        else UARTPutint((u64) x, 16);
        return *this;
    }

    UART & operator << (char x);
    UART & operator << (const char *x);
};

extern UART uPut;

#endif // UART_H_INCLUDED
