#include <new>
#include <mem.h>

void* operator new(std::size_t count) {
    if (count <= 0) count = 4096;
    if (count <= 4096 - sizeof(SmallMemNode)) return smallMemAllocator.Alloc(count);
    u32 t = 4096;
    for (u8 i = 0; i < PAGE_GROUP_SIZE_BIT; ++i, t <<= 1) {
        if (t >= count) return pageAllocator.AllocPageMem(i);
    }
    return pageAllocator.AllocPageMem(PAGE_GROUP_SIZE_BIT - 1);
}

void operator delete(void* ptr) noexcept {
    if (!smallMemAllocator.Free(ptr)) {
        pageAllocator.FreePageMem(ptr);
    }
}

void operator delete  ( void* ptr, std::size_t sz ) noexcept {
    if (!smallMemAllocator.Free(ptr)) {
        pageAllocator.FreePageMem(ptr);
    }
}

void* operator new[](std::size_t count) {
    return operator new(count);
}

void operator delete[](void* ptr) noexcept {
    operator delete(ptr);
}
