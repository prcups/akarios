#ifndef GPT_H_INCLUDED
#define GPT_H_INCLUDED

#include <disk.h>

class FileSystem;

class PartitionDisk : public Disk {
    Disk *parent;
    u64 firstBlock;
    u64 blockCount;

public:
    PartitionDisk(Disk *parent, u64 firstBlock, u64 blockCount);

    void Read(u64 blockNum, char *buf) override;
    void Write(u64 blockNum, char *buf) override;
};

class GPTPartitionTable {
    Disk *disk;
    u64 firstUsableBlock;
    u64 lastUsableBlock;
    u64 entryArrayBlock;
    u32 entryCount;
    u32 entrySize;
    bool valid;

    bool load();
    bool validateEntryArray(u32 expectedCrc);
    bool readEntry(u32 index, u8 *entry);

public:
    explicit GPTPartitionTable(Disk *disk);

    bool IsValid() const { return valid; }
    u32 GetPartitionCount() const { return valid ? entryCount : 0; }
    PartitionDisk *ReadPartition(u32 index);
};

void ScanDiskFileSystems(Disk *disk, ListItem<FileSystem*> **fileSystems,
                         u32 depth = 0);

#endif // GPT_H_INCLUDED
