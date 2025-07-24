#include <mem.h>

MMU::MMU() {
    pageTable = pageAllocator.AllocPageMem(0);
}

MMU::~MMU() {
    pageAllocator.FreePageMem(pageTable);
}

void MMU::setConfig(PTE* p, ZoneConfig& config) {
    p->p = 1;
    p->v = 1;
    p->mat = config.mat;
    p->d = config.d;
    p->nr = config.nr;
    p->nx = config.nx;
    p->plv = config.plv;
    p->rplv = config.rplv;
}

void MMU::AddItem(u64 vaddr, u64 paddr, ZoneConfig &config)
{
    PTE* p1 = (PTE*) pageTable + getPartical(vaddr, 47, 39);
    if (p1->p == 0) {
        setConfig(p1, config);
        p1->pa = (u64) pageAllocator.AllocPageMem(0) >> 12;
    }

    PTE* p2 = (PTE*) (p1->pa << 12) + getPartical(vaddr, 38, 30);
    if (p2->p == 0) {
        setConfig(p2, config);
        p2->pa = (u64) pageAllocator.AllocPageMem(0) >> 12;
    }

    PTE* p3 = (PTE*) (p2->pa << 12) + getPartical(vaddr, 29, 21);
    if (p3->p == 0) {
        setConfig(p3, config);
        p3->pa = (u64) pageAllocator.AllocPageMem(0) >> 12;
    }

    PTE* p4 = (PTE*) (p3->pa << 12) + (getPartical(vaddr, 20, 12));
    PTE* p5 = p4 + 1;
    setConfig(p4, config);
    setConfig(p5, config);
    p4->pa = (paddr >> 12);
    __ldpte_d((long) p4, 0);
    p5->pa = (paddr >> 12) + 1;
    __ldpte_d((long) p5, 1);
}

void MMU::DeleteItem(u64 vaddr)
{
    PTE* p1 = (PTE*) pageTable + getPartical(vaddr, 47, 39);
    if (p1->p == 0) return;

    PTE* p2 = (PTE*) (p1->pa << 12) + getPartical(vaddr, 38, 30);
    if (p2->p == 0) return;

    PTE* p3 = (PTE*) (p2->pa << 12) + getPartical(vaddr, 29, 21);
    if (p3->p == 0) return;

    PTE* p4 = (PTE*) (p3->pa << 12) + (getPartical(vaddr, 20, 12));
    p4->p = 0;
    PTE* p5 = p4 + 1;
    p5->p = 0;
}

u64 MMU::V2P(u64 vaddr)
{
    PTE* p1 = (PTE*) pageTable + getPartical(vaddr, 47, 39);
    if (p1->p == 0) return 0;

    PTE* p2 = (PTE*) (p1->pa << 12) + getPartical(vaddr, 38, 30);
    if (p2->p == 0) return 0;

    PTE* p3 = (PTE*) (p2->pa << 12) + getPartical(vaddr, 29, 21);
    if (p3->p == 0) return 0;

    PTE* p4 = (PTE*) (p3->pa << 12) + (getPartical(vaddr, 20, 12));
    if (p4->p == 0) return 0;
    else return (p4->pa << 12) + getPartical(vaddr, 11, 0);
}

void MMU::SetPGDL()
{
    __csrwr_d((u64) pageTable, 0x19);
}

