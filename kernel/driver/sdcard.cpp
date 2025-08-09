#include <sdcard.h>

SDCard::SDCard(void* addr, void* dma): baseAddress((u32 *)addr), dmaAddress((u32 *) dma) {
    *(baseAddress) = 1;
    *(baseAddress + 1) = 1;
    *(baseAddress + 25) = 0x3FF;

    execShortCmd(0, 0);
    if (execShortCmd(8, 0x1AA) & (1 << 6)) {
        execShortCmd(55, 0);
        if (execShortCmd(41, 0x100 + (1 << 30)) != 0x40) {
            return;
        }
    } else {
        execShortCmd(55, 0);
        if (execShortCmd(41, 0) != 0x40) {
            return;
        }
        return;
    }
    if ((getShortResponse() & (1 << 30)) && (getShortResponse() & (1 << 24))) execShortCmd(11, 0);
    execLongCmd(2, 0);
    execShortCmd(3, 0);
    rca = getShortResponse() & (((1 << 16) - 1) << 16);
    execShortCmd(7, rca);
    execShortCmd(55, rca);
    execShortCmd(6, 3);
    execShortCmd(16, 512);
    *(baseAddress + 9) = 10;
    *(baseAddress + 10) = 512;
}

void SDCard::ReadBlock(u32 addr, void* buf) {
    *(baseAddress + 11) = 1 + (1 << 14) + (1 << 15) + (1 << 16);
    u32 *s = (u32*)pageAllocator.AllocPageMem(0);
    *(s + 1) = (u64)buf;
    *(s + 2) = (u64)(baseAddress + 16);
    *(s + 3) = 128;
    *(s + 5) = 1;
    *(s + 6) = 0;
    *(dmaAddress) = (u64) s + (1 << 3);
    execShortCmd(17, addr);
    while (!(*(baseAddress + 15) & 1));
    pageAllocator.FreePageMem(s);
}

void SDCard::WriteBlock(u32 addr, void* buf) {
    *(baseAddress + 11) = 1 + (1 << 14) + (1 << 15) + (1 << 16);
    u32 *s = (u32*)pageAllocator.AllocPageMem(0);
    *(s + 1) = (u64)buf;
    *(s + 2) = (u64)(baseAddress + 16);
    *(s + 3) = 128;
    *(s + 5) = 1;
    *(s + 6) = (1 << 12);
    *(dmaAddress) = (u64) s + (1 << 3);
    execShortCmd(24, addr);
    while (!(*(baseAddress + 15) & 1));
    pageAllocator.FreePageMem(s);
}

u32 SDCard::execShortCmd(u8 cmdId, u32 arg) {
    *(baseAddress + 2) = arg;
    *(baseAddress + 3) = cmdId + (1 << 8) + (1 << 9);
    return *(baseAddress + 15);
}

u32 SDCard::execLongCmd(u8 cmdId, u32 arg) {
    *(baseAddress + 2) = arg;
    *(baseAddress + 3) = cmdId + (1 << 8) + (1 << 9) + (1 << 10);
    return *(baseAddress + 15);
}
