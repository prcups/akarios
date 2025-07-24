#include <mem.h>
#include <larchintrin.h>

void Zone::OnPageFault(u64 vaddr)
{
    space->MMUService.AddItem(vaddr, (u64) pageAllocator.AllocPageMem(1), Config);
}

void DirectZone::OnPageFault(u64 vaddr)
{
    space->MMUService.AddItem(vaddr, vaddr + Offset, Config);
}

MemSpace::MemSpace(u64 vStart, u64 vEnd) :VStart(vStart), VEnd(vEnd) {
    ZoneTree = new Tree<Zone>;
}

MemSpace::~MemSpace() {
    delete ZoneTree;
}

void MemSpace::AddZone(TNode<Zone>* t)
{
    t->val->space = this;
    if (ZoneTree->find([t](Zone *ct)->u8{
        if (t->val->VEnd < ct->VStart) return 0;
        else if (t->val->VStart > ct->VEnd) return 2;
        else return 1;
    }) == nullptr) {
        ZoneTree->insert(t, [t](Zone *ct)->bool{
            if (ct->VStart > t->val->VEnd) return 0;
            else return 1;
        });
    }
}
