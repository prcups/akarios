#ifndef PROCESS_H
#define PROCESS_H

#include <mem.h>
#include <exception.h>
#include <larchintrin.h>
#include <fat32.h>

const u16 prioRatios[8] = { 100, 200, 300, 400, 500, 600, 700, 800 };

class FileTable {
    u64 processFd = 2;
    file sdFile[100];
public:
    u64 Open(const char *filePath);
    void Close(u64 fd);
    int Read(u64 fd, u8* buf, u64 size);
    int Write(u64 fd, u8* buf, u64 size);
    int Getcwd(u8* buf, u64 size);
    int Chdir(u8*buf);
};

struct ELFHeader {
    u32 MagicNum;
    u8 BitSize;
    u8 Endian;
    u8 HeaderVer;
    u8 OSABI;
    u64 Padding;
    u16 Type;
    u16 Architecture;
    u32 ELFVersion;
    u64 ProgramEntryAddr;
    u64 PHTBAddr;
    u64 SHTBAddr;
    u32 Flags;
    u16 HeaderSize;
    u16 SizeofEntryInPHTB;
    u16 NumofEntriesInPHTB;
    u16 SizeofEntryInSHTB;
    u16 NumofEntriesInSHTB;
    u16 IndexInSHTB;
};

struct PHTB {
    u32 Type;
    u64 DataAddr;
    u64 MemAddr;
    u64 Resv;
    u64 SizeOfSegmentInFile;
    u64 SizeOfSegmentInMem;
    u32 Flags;
    u64 Align;
};

class ELFProgram {
public:
    ELFHeader *header;
    PHTB* phtb;
    ELFProgram(const char *filename);
    void ShowInfo();
    void CreateProcess();
};

class Process {
    MemSpace *space;
    u64 reg[30];
    u64 sp;
    u64 pc;

public:
    Process(u8 priority, u8 nice, void* startAddress);
    ~Process();
    u16 Id;
    Process *Next;
    u8 Priority;
    u8 Nice;
    u8 pwd[128];
    Jiffies Deadline;
    void Resume();
    void Pause();
    void SetArg(void* argc, u64 argv);
    MemSpace *GetSpace() {
        return space;
    }
    FileTable SdFileTable;
};

class ProcessController {
    u8 headBitMap = 0;
    Process *bfsHead[8];
public:
    ProcessController();
    Process *CurrentProcess;
    void HandleSchedule();
    void StopCurrentProcess();
    void InsertProcess(Process *process);
};

extern ProcessController processController;

#endif
