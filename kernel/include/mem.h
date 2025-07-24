#ifndef MEM_H
#define MEM_H

#include <util.h>
#include <uart.h>
#include <tree.h>
#include <larchintrin.h>
#include <new>

#define PAGE_GROUP_SIZE_BIT 12
#define PAGE_SIZE_BIT 12
#define PAGE_SIZE (1 << PAGE_SIZE_BIT)
#define PAGEINFO_SIZE_BIT 5

struct Page {
	Page *Prev;
	Page *Next;
	u32 RefCount;
    u32 SizeBit;
    u32 Valid;
    u32 Resv;
};

struct ZoneConfig {
    u8 mat:2;
    u8 plv:2;
    u8 rplv:1;
    u8 d:1;
    u8 nr:1;
    u8 nx:1;
};

struct PTE{
    u8 v:1;
    u8 d:1;
    u8 plv:2;
    u8 mat:2;
    u8 g:1;
    u8 p:1;
    u8 w:1;
    u8 resv2:3;
    u64 pa:36;
    u16 resv1:13;
    u8 nr:1;
    u8 nx:1;
    u8 rplv:1;
};

class MMU {
    void *pageTable;
    inline void setConfig(PTE* p, ZoneConfig &config);
public:
    MMU();
    ~MMU();
    void SetPGDL();
    void AddItem(u64 vaddr, u64 paddr, ZoneConfig &config);
    void DeleteItem(u64 vaddr);
    u64 V2P(u64 vaddr);
};

class MemSpace;

class Zone {
public:
    Zone(u64 start, u64 end, ZoneConfig config) :VStart(start), VEnd(end), Config(config){}
	virtual void OnPageFault(u64 vaddr);
    virtual ~Zone() {}
	u64 VStart;
	u64 VEnd;
    ZoneConfig Config;
    MemSpace *space;
};

class MemSpace {
public:
	u64 VStart;
	u64 VEnd;
	MMU MMUService;
	Tree<Zone> *ZoneTree;
    MemSpace(u64 vStart, u64 vEnd);
    ~MemSpace();
    void AddZone(TNode<Zone> *t);
    void DeleteZone(Zone *t);
};

class DirectZone : public Zone {
public:
    u64 Offset;
    DirectZone(u64 vstart, u64 vend, u64 offset, ZoneConfig config): Zone(vstart, vend, config), Offset(offset){}
    virtual void OnPageFault(u64 vaddr) override;
};

class PageAllocator {
    Page *pageInfo;
    u64 pageAreaStart;
	Page* buddyHeadList[PAGE_GROUP_SIZE_BIT];
    void setupPage(Page* t, u8 sizeBit) {
      t->SizeBit = sizeBit;
    }
    void addPageToBuddy(Page* t);
    void deletePageFromBuddy(Page *t);
    void setPageInfo(u64 pageInfoAddress, u64 pageAreaStart);
    Page* getBuddyPage(Page *t) {
      return (Page *)((u64)t ^ (1 << (t->SizeBit + PAGEINFO_SIZE_BIT)));
    }
public:
	PageAllocator();
	u64 PageToAddr(Page* t) {
      return (((t - pageInfo) << PAGE_SIZE_BIT) + (u64) pageAreaStart);
    }
	Page* AddrToPage(u64 t) {
      return pageInfo + ((t - pageAreaStart) >> PAGE_SIZE_BIT);
    }
	Page* AllocPage(u8 sizeBit);
    void* AllocPageMem(u8 sizeBit) {
        Page * t = AllocPage(sizeBit);
        return (void*) PageToAddr(t);
    }
	void FreePage(Page* t);
    void FreePageMem(void* addr) {
        Page *t = AddrToPage((u64) addr);
        FreePage(t);
    }
    void AddArea(u64 start, u64 end, bool isMaskedAsIllegal);
    void ListPage();
};

struct SlabNode {
    void* Start;
    u16 Size;
    SlabNode *Next;
};

class Slab {
protected:
    void* pageMem;
public:
    virtual void* Malloc(u16 size) = 0;
    virtual bool Free(void* addr) = 0;
};

class SlabNodeArea: public Slab {
    u8 bitMap[28];
    bool used;
public:
    SlabNodeArea();
    SlabNodeArea* Next;
    void* Malloc(u16 size) override;
    bool Free(void* addr) override;
};

class SlabArea: public Slab{
    SlabNode *availableMem;
    SlabNode *usedMem;
    friend class SlabAllocator;
    bool used;
public:
    SlabArea();
    SlabArea* Next;
    void* Malloc(u16 size) override;
    bool Free(void* addr) override;
};

class SlabNodeAllocator {
    SlabNodeArea *slabNodeZonePtr;
    u64 zoneNum;
    u64 nodeNum;
public:
    SlabNodeAllocator();
    SlabNode* AllocNode();
    void FreeNode(SlabNode *node);
};

class SlabAllocator {
    SlabArea *defaultSlabZonePtr;
public:
    SlabAllocator();
    void* Malloc(u16 size);
    bool Free(void* addr);
    void ListZone();
};

extern void* KernelEnd;
extern const u64 vaddrEnd;

extern PageAllocator pageAllocator;

extern SlabNodeArea slabNodeZone;
extern SlabNodeAllocator slabNodeAllocator;
extern SlabArea defaultSlabZone;
extern SlabAllocator defaultSlabAllocator;

#endif
