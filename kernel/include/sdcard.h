#ifndef SDCARD_H
#define SDCARD_H

#include <util.h>
#include <uart.h>
#include <mem.h>
#include <disk.h>

class SDCard : public Disk {
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
    void Read(u64 blockNum, char *buf) override;
    void Write(u64 blockNum, char *buf) override;
};

extern SDCard sdcard;

#endif
