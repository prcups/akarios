
#include <mem.h>

SlabAllocator::SlabAllocator(){
    defaultSlabZonePtr = &defaultSlabZone;
}

void * SlabAllocator::Malloc(u16 size){
    SlabArea* zone = defaultSlabZonePtr;
    void *addr = zone->Malloc(size);
    while (addr == nullptr && zone->Next != nullptr) {
        zone = zone->Next;
        uPut << zone->pageMem << '\n';
        addr = zone->Malloc(size);
    }
    return addr;
}

bool SlabAllocator::Free(void* addr){
    SlabArea* zone = defaultSlabZonePtr;
    bool status = zone->Free(addr);
    while (status == false && zone->Next != nullptr) {
        zone = zone->Next;
        status = zone->Free(addr);
    }
    return status;
}

void SlabAllocator::ListZone()
{
    for (SlabArea* zone = defaultSlabZonePtr; zone != nullptr; zone = zone->Next) {
        if (zone->used == true) {
            uPut << zone->pageMem << ":\n";
            if (zone->availableMem != nullptr) {
                uPut << "Available SlabNode:\n";
                for (SlabNode *pt = zone->availableMem; pt != nullptr; pt = pt->Next) {
                    uPut << pt->Start << " " << pt->Size << '\n';
                }
            }
            if (zone->usedMem != nullptr) {
                uPut << "used SlabNode:\n";
                for (SlabNode *pt = zone->usedMem; pt != nullptr; pt = pt->Next) {
                    uPut << pt->Start << " " << pt->Size << '\n';
                }
            }
        }
    }
}

SlabArea::SlabArea(): used(false){
    Next = nullptr;
    pageMem = nullptr;
}

void * SlabArea::Malloc(u16 size) {
    if (size > 4096 - sizeof(SlabArea)) return nullptr;
    if (used == false) {
        pageMem = pageAllocator.AllocPageMem(0);
        uPut << pageMem << '\n';
        Next = new ((void*) pageMem) SlabArea;
        availableMem = slabNodeAllocator.AllocNode();
        availableMem->Start = (SlabArea*) pageMem + 1;
        availableMem->Size = 4096 - sizeof(SlabArea);
        used = true;
    }
    SlabNode *nodePtr = availableMem;
    SlabNode *pre = nullptr;
    while (nodePtr != nullptr) {
        if (nodePtr->Size == size) {
            if (pre == nullptr) {
                availableMem = nodePtr->Next;
            } else {
                pre->Next = nodePtr->Next;
            }
            nodePtr->Next = usedMem;
            usedMem = nodePtr;
            return nodePtr->Start;
        } else if (nodePtr->Size > size) {
            SlabNode *newNode = slabNodeAllocator.AllocNode();
            newNode->Start = nodePtr->Start;
            newNode->Size = size;
            newNode->Next = usedMem;
            usedMem = newNode;
            nodePtr->Start = (void*)((u64) nodePtr->Start + size);
            nodePtr->Size = nodePtr->Size - size;
            return newNode->Start;
        }
        pre = nodePtr;
        nodePtr = nodePtr->Next;
    }
    return nullptr;
}

bool SlabArea::Free(void* addr){
    if (used == 0 || ((u64) addr & 0xFFFFFFFFFFFFF000) != (u64) pageMem) return false;
    SlabNode *node = usedMem;
    SlabNode *p = nullptr;
    while (node != nullptr) {
        if (node->Start == addr) {
            if (p == nullptr) {
                usedMem = node->Next;
            } else {
                p->Next = node->Next;
            }
            node->Next = availableMem;
            availableMem = node;
            return true;
        }
        p = node;
        node = node->Next;
    }
    return false;
}

SlabNodeAllocator::SlabNodeAllocator(): zoneNum(1), nodeNum(0){
    slabNodeZonePtr = &slabNodeZone;
}

SlabNode * SlabNodeAllocator::AllocNode() {
    SlabNodeArea* zone = slabNodeZonePtr;
    void* node = zone->Malloc(sizeof(SlabNode));
    while (node == nullptr && zone->Next != nullptr) {
        zone = reinterpret_cast<SlabNodeArea*>(zone->Next);
        node = zone->Malloc(sizeof(SlabNode));
    }
    return (SlabNode*) node;
}

void SlabNodeAllocator::FreeNode(SlabNode* node) {
    SlabNodeArea* zone = slabNodeZonePtr;
    bool status = zone->Free(node);
    while (status == false && zone->Next != nullptr) {
        zone = reinterpret_cast<SlabNodeArea*>(zone->Next);
        status = zone->Free(node);
    }
}

SlabNodeArea::SlabNodeArea(): used(false){
    pageMem = pageAllocator.AllocPageMem(0);
    for (u8 i = 0; i < 28; ++i) bitMap[i] = 0;
}

void * SlabNodeArea::Malloc(u16 size) {
    if (size != sizeof(SlabNode)) return nullptr;
    if (used == false) {
        pageMem = pageAllocator.AllocPageMem(0);
        Next = new ((void*) ((u64) pageMem + 4032)) SlabNodeArea;
        used = true;
    }
    for (u8 i = 0; i < 28; ++i) {
        for (u8 j = 0; j < 8; ++j) {
            if ((bitMap[i] & (1 << j)) == 0) {
                bitMap[i] |= (1 << j);
                return (void *)((u64)pageMem + (i * 8 + j) * size);
            }
        }
    }
    return nullptr;
}


bool SlabNodeArea::Free(void* addr) {
    if (((u64) addr & 0xFFFFFFFFFFFFF000) != (u64) pageMem) return false;
    u16 t = (u64) addr / sizeof(SlabNode);
    u8 i = t / 8;
    u8 j = t % 8;
    if (bitMap[i] & (1 << j)) {
        bitMap[i] &= (~(1 << j));
        return true;
    }
    return false;
}
