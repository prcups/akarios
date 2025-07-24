#include <util.h>

void Mutex::Lock() {
    u32 t = 1;
    while (t) {
        __asm__(
            "amswap.w %0, %1, %2"
            :"+r"(t)
            :"r"(1), "r"(&locked)
        );
    }
}

void Mutex::Unlock() {
    __asm__(
        "addi.w %0, $zero, 0"
        :"=r"(locked)
    );
}
