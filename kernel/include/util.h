#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <compare>

using u64 = unsigned long long;
using u32 = unsigned int;
using u16 = unsigned short;
using u8 = unsigned char;

namespace KernelUtil {
    template <class T>
    void Swap(T & a, T & b);

    u32 Strlen(const char* s);

    template <class T>
    inline T Max(T a, T b) {
        return a > b ? a : b;
    }

    template <class T>
    inline T Min(T a, T b) {
        return a < b ? a : b;
    }
    int Memcmp(const void*, const void*, u32);
    void* Memmove(void*, const void*, u32);
    void* Memset(void*, int, u32);
    char* Safestrcpy(char*, const char*, int);
    int Strncmp(const char*, const char*, u32);
    char* Strncpy(char*, const char*, int);
    void* Memcpy(void *dst, const void *src, u32 n);
    char* Strchr(const char* str, char ch);
    char ToUpper(char c);
    int IsLower(int c);
};

u64 getCPUCFG(u64 input);

template <class T>
inline T getPartical(T val, u8 high, u8 low) {
    return (val >> low) % (1 << (high - low + 1));
}

inline u64 upAlign(u64 addr, u8 sizeBit) {
    return addr + (1ull << sizeBit) & (~((1ull << sizeBit) - 1));
}

class Mutex {
    u32 locked = 0;
public:
    void Lock();
    void Unlock();
};

class Jiffies{
    u64 val;
    static u64 sysJiffies;
    friend class Exception;
public:
    friend auto operator <=> (const Jiffies & a, const Jiffies & b) {
        Jiffies now = GetJiffies();
        return (a.val - now.val) <=> (b.val - now.val);
    }
    friend auto operator + (const Jiffies & a, const Jiffies & b) {
        return Jiffies(a.val + b.val);
    }
    operator unsigned int() {
        return val;
    }
    static Jiffies GetJiffies() {
        return Jiffies(sysJiffies);
    }
    Jiffies(u64 val = 0):val(val){}
};

#endif // UTIL_H_INCLUDED
