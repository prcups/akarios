#include <mem.h>

PageAllocator::PageAllocator(){
    setPageInfo(upAlign((u64) (&KernelEnd), PAGEINFO_SIZE_BIT), 0);

    u64 mprEnd = upAlign((u64) pageInfo + ((0xFFFFFFFF - pageAreaStart + 1) >> (PAGE_SIZE_BIT - PAGEINFO_SIZE_BIT)), PAGE_SIZE_BIT) - 1;

    for (Page *i = pageInfo; i < pageInfo + ((0x100000000 - pageAreaStart) >> PAGE_SIZE_BIT); ++i) {
        i->Valid = 0;
    }

    AddArea(0, mprEnd, 1);
    AddArea(mprEnd + 1, 0xFFFFFFFF, 0);
    AddArea(0x10000000, 0x7FFFFFFF, 1);
    AddArea(0x80000000, 0xFFFFFFFF, 0);
}

void PageAllocator::setPageInfo(u64 pageInfoAddress, u64 pageAreaStart)
{
    pageInfo = (Page *) pageInfoAddress;
    this->pageAreaStart = pageAreaStart;
}

void PageAllocator::AddArea(u64 start, u64 end, bool isMaskedAsIllegal)
{
    u64 pt = start;
    u8 currentPageSizeBit = 0;
    Page *t;
    while (1) {
        while ((pt & (1 << (currentPageSizeBit + PAGE_SIZE_BIT))) == 0 && currentPageSizeBit < PAGE_GROUP_SIZE_BIT - 1) ++currentPageSizeBit;
        if (pt + (1 << (currentPageSizeBit + PAGE_SIZE_BIT)) - 1 > end) break;
        t = AddrToPage(pt);
        setupPage(t, isMaskedAsIllegal ? PAGE_GROUP_SIZE_BIT : currentPageSizeBit);
        if (!isMaskedAsIllegal) addPageToBuddy(t);
        pt += (1 << (currentPageSizeBit + PAGE_SIZE_BIT));
    }
    for (; pt <= end;
         pt += (1 << (currentPageSizeBit + PAGE_SIZE_BIT))) {

        while (pt + (1 << (currentPageSizeBit + PAGE_SIZE_BIT)) - 1 > end) --currentPageSizeBit;
        t = AddrToPage(pt);
        setupPage(t, isMaskedAsIllegal ? PAGE_GROUP_SIZE_BIT : currentPageSizeBit);
        if (!isMaskedAsIllegal) addPageToBuddy(t);
    }
}

void PageAllocator::addPageToBuddy(Page* t)
{
    if (buddyHeadList[t->SizeBit]) {
        t->Next = buddyHeadList[t->SizeBit];
        buddyHeadList[t->SizeBit]->Prev = t;
    }
    buddyHeadList[t->SizeBit] = t;
    t->Valid = 1;
}

void PageAllocator::deletePageFromBuddy(Page* t)
{
    if (t->Next) t->Next->Prev = t->Prev;
    if (t->Prev) t->Prev->Next = t->Next;
    else buddyHeadList[t->SizeBit] = t->Next;
    t->Next = t->Prev = nullptr;
    t->Valid = 0;
}

Page* PageAllocator::AllocPage(u8 sizeBit) {
    Page *p = nullptr;
    for (u8 i = sizeBit; i < PAGE_GROUP_SIZE_BIT; ++i) {
        if (buddyHeadList[i] != nullptr) {
            p = buddyHeadList[i];
            break;
        }
    }
    deletePageFromBuddy(p);
    Page *buddy;
    while (p->SizeBit > sizeBit) {
        --p->SizeBit;
        buddy = getBuddyPage(p);
        setupPage(buddy, p->SizeBit);
        addPageToBuddy(buddy);
    }
    ++p->RefCount;
    return p;
}

void PageAllocator::FreePage(Page* t)
{
    --t->RefCount;
    if (t->RefCount != 0) return;
    Page *buddy;
    for (; t->SizeBit < PAGE_GROUP_SIZE_BIT; ++t->SizeBit) {
        buddy = getBuddyPage(t);
        if (buddy->Valid == 1) {
            deletePageFromBuddy(buddy);
            setupPage(buddy, t->SizeBit);
            t = buddy - t > 0 ? t : buddy;
        } else {
            break;
        }
    }
    addPageToBuddy(t);
}

void PageAllocator::ListPage() {
    for (u8 i = 0; i < PAGE_GROUP_SIZE_BIT; ++i) {
        u64 c = 0;
        for (Page *t = buddyHeadList[i]; t != nullptr; t = t->Next) {
            ++c;
        }
        uPut << "2 ^ " << i << " = " << c << '\n';
    }
}
