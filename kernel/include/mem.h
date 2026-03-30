#ifndef MEM_H
#define MEM_H

#include <util.h>
#include <uart.h>
#include <tree.h>
#include <larchintrin.h>
#include <new>
#include <acpi.h>

#define PAGE_GROUP_SIZE_BIT 12
#define PAGE_SIZE_BIT 12
#define PAGE_SIZE (1 << PAGE_SIZE_BIT)
#define PAGEINFO_SIZE_BIT 5
#define PAGEINFO_SIZE (1 << PAGEINFO_SIZE_BIT)

enum EFIMemType {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType
};

struct MemMapDesp {
    EFIMemType Type;
    u64 PStart;
    u64 VStart;
    u64 NumOfPages;
    u64 Attr;
};

struct BootInfo {
    XSDP* XsdpPtr;
    MemMapDesp* MemMap;
    u32 DespSize;
    u32 MemMapSize;
};

struct Page {
	Page *Prev;
	Page *Next;
	u32 RefCount;
    u32 SizeBit;
    u32 InBuddy;
    u32 resv;
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
    void setConfig(PTE* p, ZoneConfig &config);
    void initPTE(PTE* p);
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
    u64 Paddr;
    DirectZone(u64 vstart, u64 vend, u64 paddr, ZoneConfig config): Zone(vstart, vend, config), Paddr(paddr){}
    virtual void OnPageFault(u64 vaddr) override;
};

class PageAllocator {
    Page *mprStart;
    u64 pageAreaStart, mprEnd;
	Page* buddyHeadList[PAGE_GROUP_SIZE_BIT];
    bool isIllegal(EFIMemType type);
    void addPageToBuddy(Page* t);
    void deletePageFromBuddy(Page *t);
    Page* getBuddyPage(Page *t);
    void initPage(Page* t, u32 sizeBit);
public:
    void Init(BootInfo &info);
	u64 PageToAddr(Page* t);
	Page* AddrToPage(u64 t);
	Page* AllocPage(u8 sizeBit);
    void* AllocPageMem(u8 sizeBit);
	void FreePage(Page* t);
    void FreePageMem(void* addr);
    void AddArea(u64 start, u64 end, bool isMaskedAsIllegal);
    void ListPage();
};

struct SmallMemNode {
    void* Mem;
    SmallMemNode* Next;
    SmallMemNode* Prev;
    u64 Size;
};

class SmallMemAllocator {
    SmallMemNode* free = nullptr;
    SmallMemNode* used = nullptr;
public:
    void* Alloc(u32 size);
    bool Free(void* mem);
    void List();
};

extern void* KernelEnd;
extern const u64 vaddrEnd;

extern PageAllocator pageAllocator;
extern SmallMemAllocator smallMemAllocator;

#endif
