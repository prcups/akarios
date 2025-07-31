#include <mem.h>

void PageAllocator::Init(BootInfo& info)
{
    MemMapDesp *pt;
    pageAreaStart = upAlign((u64) (&KernelEnd), PAGE_SIZE_BIT);
    u64 memMapEnd = (u64) info.MemMap + info.MemMapSize * info.DespSize - 1;
    u64 memSize = 0, maxMemPageNum = 0, maxMemStart;
    u64 pageSectionStart, pageSectionEnd, mprSize;

    for (u32 i = 0; i < PAGE_GROUP_SIZE_BIT; ++i)
        buddyHeadList[i] = nullptr;

    for (u32 i = 0; i < info.MemMapSize; ++i)
    {
        pt = (MemMapDesp *)((u64)(info.MemMap) + i * info.DespSize);
        pageSectionStart = pt->PStart;
        pageSectionEnd = pt->PStart + (pt->NumOfPages << PAGE_SIZE_BIT) - 1;
        if (pt->PStart + (pt->NumOfPages << PAGE_SIZE_BIT) > memSize)
            memSize = pt->PStart + (pt->NumOfPages << PAGE_SIZE_BIT);
        if (pt->NumOfPages > maxMemPageNum
            && pageSectionStart >= pageAreaStart
            && ((u64) info.MemMap > pageSectionEnd
                || memMapEnd < pageSectionStart)
            && !isIllegal(pt->Type)
            )
        {
            maxMemPageNum = pt->NumOfPages;
            maxMemStart = pt->PStart;
        }
    }

    mprSize = ((memSize - pageAreaStart) >> (PAGE_SIZE_BIT - PAGEINFO_SIZE_BIT));
    if (maxMemPageNum * PAGE_SIZE >= mprSize)
        mprStart = (Page *) maxMemStart;
    else
    {
        uPut << "Can't allocate page management area.";
        while (1);
    }

    mprEnd = (u64) mprStart + mprSize - 1;

    for (int i = 0; i < info.MemMapSize; ++i)
    {
        pt = (MemMapDesp *)((u64)(info.MemMap) + i * info.DespSize);
        pageSectionStart = pt->PStart;
        pageSectionEnd = pt->PStart + (pt->NumOfPages << PAGE_SIZE_BIT) - 1;
        if (pageSectionEnd < pageAreaStart) continue;
        if (pageSectionStart < pageAreaStart)
            AddArea(pageAreaStart, pageSectionEnd, isIllegal(pt->Type));
        else if (pageSectionStart == (u64) mprStart)
        {
            AddArea((u64) mprStart, mprEnd, true);
            if (mprEnd + 1 < pageSectionEnd)
                AddArea(upAlign(mprEnd + 1, PAGE_SIZE_BIT), pageSectionEnd, isIllegal(pt->Type));
        }
        else AddArea(pageSectionStart, pageSectionEnd, isIllegal(pt->Type));
    }
}

bool PageAllocator::isIllegal(EFIMemType type)
{
    if (type == EfiLoaderCode
        || type == EfiLoaderData
        || type == EfiBootServicesCode
        || type == EfiBootServicesData
        || type == EfiConventionalMemory
        || type == EfiUnacceptedMemoryType
    ) return false;
    else return true;
}

void PageAllocator::initPage(Page* t, u32 sizeBit)
{
    t->SizeBit = sizeBit;
    t->InBuddy = 0;
    t->Next = nullptr;
    t->Prev = nullptr;
    t->RefCount = 0;
}

void PageAllocator::AddArea(u64 start, u64 end, bool isMaskedAsIllegal)
{
    u64 pt = start;
    u8 currentPageSizeBit = 0;
    Page *t;
    while (1) {
        while (((pt - pageAreaStart) & (1 << (currentPageSizeBit + PAGE_SIZE_BIT))) == 0 && currentPageSizeBit < PAGE_GROUP_SIZE_BIT - 1) ++currentPageSizeBit;
        if (pt + (1 << (currentPageSizeBit + PAGE_SIZE_BIT)) - 1 > end) break;
        t = AddrToPage(pt);
        initPage(t, isMaskedAsIllegal ? PAGE_GROUP_SIZE_BIT : currentPageSizeBit);
        if (!isMaskedAsIllegal) addPageToBuddy(t);
        pt += (1 << (currentPageSizeBit + PAGE_SIZE_BIT));
    }
    for (; pt <= end;
         pt += (1 << (currentPageSizeBit + PAGE_SIZE_BIT))) {

        while (pt + (1 << (currentPageSizeBit + PAGE_SIZE_BIT)) - 1 > end) --currentPageSizeBit;
        t = AddrToPage(pt);
        initPage(t, isMaskedAsIllegal ? PAGE_GROUP_SIZE_BIT : currentPageSizeBit);
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
    t->InBuddy = 1;
}

void PageAllocator::deletePageFromBuddy(Page* t)
{
    if (t->Next) t->Next->Prev = t->Prev;
    if (t->Prev) t->Prev->Next = t->Next;
    else buddyHeadList[t->SizeBit] = t->Next;
    t->Next = t->Prev = nullptr;
    t->InBuddy = 0;
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
        initPage(buddy, sizeBit);
        addPageToBuddy(buddy);
    }
    ++p->RefCount;
    return p;
}

void PageAllocator::FreePage(Page* t)
{
    if (t < mprStart
        || (u64) t > mprEnd
        || t->SizeBit == PAGE_GROUP_SIZE_BIT
    ) return;
    --t->RefCount;
    if (t->RefCount != 0) return;
    Page *buddy;
    for (; t->SizeBit < PAGE_GROUP_SIZE_BIT - 1; ++t->SizeBit) {
        buddy = getBuddyPage(t);
        if (buddy == nullptr || buddy->InBuddy == 0) break;
        deletePageFromBuddy(buddy);
        buddy->SizeBit = t->SizeBit;
        t = buddy - t > 0 ? t : buddy;
    }
    addPageToBuddy(t);
}

void PageAllocator::ListPage() {
    for (u8 i = 0; i < PAGE_GROUP_SIZE_BIT; ++i) {
        u64 c = 0;
        for (Page *t = buddyHeadList[i]; t != nullptr; t = t->Next) {
            ++c;
        }
        uPut << "2 ^ " << i << " = " << c << "\r\n";
    }
}

Page * PageAllocator::getBuddyPage(Page* t)
{
    Page *page = (Page *)((u64)t ^ (1 << (t->SizeBit + PAGEINFO_SIZE_BIT)));
    if ((u64) page > mprEnd) return nullptr;
    return page;
}

u64 PageAllocator::PageToAddr(Page* t)
{
    return (((t - mprStart) << PAGE_SIZE_BIT) + (u64) pageAreaStart);
}

Page * PageAllocator::AddrToPage(u64 t)
{
    return mprStart + ((t - pageAreaStart) >> PAGE_SIZE_BIT);
}

void * PageAllocator::AllocPageMem(u8 sizeBit)
{
    Page * t = AllocPage(sizeBit);
    return (void*) PageToAddr(t);
}

void PageAllocator::FreePageMem(void* addr)
{
    Page *t = AddrToPage((u64) addr);
    FreePage(t);
}

