
#include <mem.h>

void* SmallMemAllocator::Alloc(u32 size)
{
    if (size > PAGE_SIZE - sizeof(SmallMemNode)) return nullptr;
    u32 realSize = upAlign(size, 3);
    SmallMemNode *pt, *newpt;
    for (pt = free; pt != nullptr; pt = pt->Next)
        if (pt->Size >= realSize) break;
    if (pt == nullptr)
    {
        pt = (SmallMemNode*) pageAllocator.AllocPageMem(1);
        pt->Next = free;
        pt->Prev = nullptr;
        pt->Mem = (void*)(pt + 1);
        pt->Size = PAGE_SIZE - sizeof(SmallMemNode);
    }
    if (realSize == pt->Size)
    {
        if (pt->Next) pt->Next->Prev = pt->Prev;
        if (pt->Prev) pt->Prev->Next = pt->Next;
        else free = pt->Next;
    }
    else
    {
        newpt = (SmallMemNode*)((u64) pt->Mem + realSize);
        newpt->Size = pt->Size - realSize - sizeof(SmallMemNode);
        newpt->Mem = (void*)(newpt + 1);
        newpt->Next = pt->Next;
        newpt->Prev = pt->Prev;
        if (newpt->Next) newpt->Next->Prev = newpt;
        if (newpt->Prev) newpt->Prev->Next = newpt;
        else free = newpt;
    }
    pt->Size = realSize;
    pt->Next = used;
    pt->Prev = nullptr;
    used = pt;
    return pt->Mem;
}

bool SmallMemAllocator::Free(void* mem)
{
    SmallMemNode *pt;
    for (pt = used; pt != nullptr; pt = pt->Next)
        if (mem == pt->Mem) break;
    if (pt == nullptr) return false;
    if (pt->Next) pt->Next->Prev = pt->Prev;
    if (pt->Prev) pt->Prev->Next = pt->Next;
    else used = pt->Next;
    pt->Next = free;
    pt->Prev = nullptr;
    free = pt;
    return true;
}

void SmallMemAllocator::List()
{
    SmallMemNode *pt;
    uPut << "free: \r\n";
    for (pt = free; pt != nullptr; pt = pt->Next)
        uPut << pt->Mem << ": " << pt->Size << "\r\n";
    uPut << "used: \r\n";
    for (pt = used; pt != nullptr; pt = pt->Next)
        uPut << pt->Mem << ": " << pt->Size << "\r\n";
}
