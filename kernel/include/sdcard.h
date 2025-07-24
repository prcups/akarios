#ifndef SDCARD_H
#define SDCARD_H

#include <util.h>
#include <uart.h>
#include <mem.h>

class SDCard {
    u32 *baseAddress;
    u32 *dmaAddress;
    u32 rca;
    u32 execShortCmd(u8 cmdId, u32 arg);
    u32 execLongCmd(u8 cmdId, u32 arg);
    u32 getShortResponse() {
        return *(baseAddress + 5);
    }
public:
    SDCard(void* addr, void* dma);
    void ReadBlock(u32 addr, void* buf);
    void WriteBlock(u32 addr, void* buf);
};

extern SDCard sdcard;

#endif
