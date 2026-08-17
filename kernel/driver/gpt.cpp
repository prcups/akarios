#include <gpt.h>
#include <fat32.h>
#include <filesystem.h>

#define DISK_BLOCK_SIZE 512
#define GPT_HEADER_BLOCK 1
#define GPT_MIN_HEADER_SIZE 92
#define GPT_MIN_ENTRY_SIZE 128
#define GPT_MAX_ENTRY_SIZE 4096
#define GPT_MAX_ENTRY_COUNT 4096
#define GPT_MAX_RECURSION_DEPTH 8

namespace {
u32 readLe32(const u8 *data) {
    return (u32)data[0] |
           ((u32)data[1] << 8) |
           ((u32)data[2] << 16) |
           ((u32)data[3] << 24);
}

u64 readLe64(const u8 *data) {
    return (u64)readLe32(data) | ((u64)readLe32(data + 4) << 32);
}

u32 updateCrc32(u32 crc, const u8 *data, u32 size) {
    for (u32 i = 0; i < size; ++i) {
        crc ^= data[i];
        for (u8 bit = 0; bit < 8; ++bit) {
            u32 mask = 0 - (crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320u & mask);
        }
    }
    return crc;
}

bool isZeroGuid(const u8 *guid) {
    for (u8 i = 0; i < 16; ++i) {
        if (guid[i] != 0) {
            return false;
        }
    }
    return true;
}

void appendFileSystem(ListItem<FileSystem*> **fileSystems, FileSystem *fileSystem) {
    auto *item = new ListItem<FileSystem*>(fileSystem);
    if (*fileSystems == nullptr) {
        *fileSystems = item;
        return;
    }

    ListItem<FileSystem*> *tail = *fileSystems;
    while (tail->Next != nullptr) {
        tail = tail->Next;
    }
    tail->Next = item;
}
}

PartitionDisk::PartitionDisk(Disk *parent, u64 firstBlock, u64 blockCount)
    : parent(parent), firstBlock(firstBlock), blockCount(blockCount) {}

void PartitionDisk::Read(u64 blockNum, char *buf) {
    if (blockNum >= blockCount || firstBlock + blockNum < firstBlock) {
        KernelUtil::Memset(buf, 0, DISK_BLOCK_SIZE);
        return;
    }
    parent->Read(firstBlock + blockNum, buf);
}

void PartitionDisk::Write(u64 blockNum, char *buf) {
    if (blockNum >= blockCount || firstBlock + blockNum < firstBlock) {
        return;
    }
    parent->Write(firstBlock + blockNum, buf);
}

GPTPartitionTable::GPTPartitionTable(Disk *disk)
    : disk(disk), firstUsableBlock(0), lastUsableBlock(0),
      entryArrayBlock(0), entryCount(0), entrySize(0), valid(false) {
    valid = load();
}

bool GPTPartitionTable::load() {
    if (disk == nullptr) {
        return false;
    }

    u8 block[DISK_BLOCK_SIZE];
    disk->Read(GPT_HEADER_BLOCK, (char *)block);
    static const u8 signature[8] = {'E', 'F', 'I', ' ', 'P', 'A', 'R', 'T'};
    if (KernelUtil::Memcmp(block, signature, sizeof(signature)) != 0) {
        return false;
    }

    u32 headerSize = readLe32(block + 12);
    u32 headerCrc = readLe32(block + 16);
    if (headerSize < GPT_MIN_HEADER_SIZE || headerSize > DISK_BLOCK_SIZE ||
        readLe32(block + 20) != 0 || readLe64(block + 24) != GPT_HEADER_BLOCK) {
        return false;
    }

    block[16] = 0;
    block[17] = 0;
    block[18] = 0;
    block[19] = 0;
    if (~updateCrc32(0xFFFFFFFFu, block, headerSize) != headerCrc) {
        return false;
    }

    firstUsableBlock = readLe64(block + 40);
    lastUsableBlock = readLe64(block + 48);
    entryArrayBlock = readLe64(block + 72);
    entryCount = readLe32(block + 80);
    entrySize = readLe32(block + 84);
    u32 entryArrayCrc = readLe32(block + 88);

    if (firstUsableBlock > lastUsableBlock || entryArrayBlock == 0 ||
        entryCount == 0 || entryCount > GPT_MAX_ENTRY_COUNT ||
        entrySize < GPT_MIN_ENTRY_SIZE || entrySize > GPT_MAX_ENTRY_SIZE ||
        (entrySize & 7) != 0) {
        return false;
    }

    u64 entryArrayBlocks = ((u64)entryCount * entrySize +
                            DISK_BLOCK_SIZE - 1) / DISK_BLOCK_SIZE;
    if (entryArrayBlock > ~0ull - entryArrayBlocks) {
        return false;
    }

    return validateEntryArray(entryArrayCrc);
}

bool GPTPartitionTable::validateEntryArray(u32 expectedCrc) {
    u64 bytesLeft = (u64)entryCount * entrySize;
    u64 blockNum = entryArrayBlock;
    u32 crc = 0xFFFFFFFFu;
    u8 block[DISK_BLOCK_SIZE];

    while (bytesLeft != 0) {
        disk->Read(blockNum++, (char *)block);
        u32 bytesInBlock = bytesLeft < DISK_BLOCK_SIZE ?
                           (u32)bytesLeft : DISK_BLOCK_SIZE;
        crc = updateCrc32(crc, block, bytesInBlock);
        bytesLeft -= bytesInBlock;
    }
    return ~crc == expectedCrc;
}

bool GPTPartitionTable::readEntry(u32 index, u8 *entry) {
    if (!valid || index >= entryCount) {
        return false;
    }

    u64 byteOffset = (u64)index * entrySize;
    u64 blockNum = entryArrayBlock + byteOffset / DISK_BLOCK_SIZE;
    u32 blockOffset = byteOffset % DISK_BLOCK_SIZE;
    u32 bytesRead = 0;
    u8 block[DISK_BLOCK_SIZE];

    while (bytesRead < GPT_MIN_ENTRY_SIZE) {
        disk->Read(blockNum++, (char *)block);
        u32 bytesAvailable = DISK_BLOCK_SIZE - blockOffset;
        u32 bytesNeeded = GPT_MIN_ENTRY_SIZE - bytesRead;
        u32 copySize = bytesAvailable < bytesNeeded ? bytesAvailable : bytesNeeded;
        KernelUtil::Memcpy(entry + bytesRead, block + blockOffset, copySize);
        bytesRead += copySize;
        blockOffset = 0;
    }
    return true;
}

PartitionDisk *GPTPartitionTable::ReadPartition(u32 index) {
    u8 entry[GPT_MIN_ENTRY_SIZE];
    if (!readEntry(index, entry) || isZeroGuid(entry)) {
        return nullptr;
    }

    u64 firstBlock = readLe64(entry + 32);
    u64 lastBlock = readLe64(entry + 40);
    if (firstBlock < firstUsableBlock || lastBlock > lastUsableBlock ||
        firstBlock > lastBlock || lastBlock == ~0ull) {
        return nullptr;
    }
    return new PartitionDisk(disk, firstBlock, lastBlock - firstBlock + 1);
}

void ScanDiskFileSystems(Disk *disk, ListItem<FileSystem*> **fileSystems,
                         u32 depth) {
    if (disk == nullptr || fileSystems == nullptr ||
        depth > GPT_MAX_RECURSION_DEPTH) {
        return;
    }

    if (Fat32::Probe(disk)) {
        appendFileSystem(fileSystems, new Fat32(disk));
        return;
    }

    GPTPartitionTable table(disk);
    if (!table.IsValid()) {
        return;
    }

    for (u32 i = 0; i < table.GetPartitionCount(); ++i) {
        PartitionDisk *partition = table.ReadPartition(i);
        if (partition != nullptr) {
            ScanDiskFileSystems(partition, fileSystems, depth + 1);
        }
    }
}
