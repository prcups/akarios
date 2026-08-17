#include <fat32.h>
#include <util.h>
#include <string.h>
#include <mem.h>

namespace {
u16 readLe16(const u8 *data) {
    return (u16)data[0] | ((u16)data[1] << 8);
}

u32 readLe32(const u8 *data) {
    return (u32)data[0] |
           ((u32)data[1] << 8) |
           ((u32)data[2] << 16) |
           ((u32)data[3] << 24);
}
}

void Fat32::toSfn(char *destName, const char *srcName) {
    char *dest = destName;
    const char *extDot;
    const char *p;
    int nameLen;
    int extExisted;

    KernelUtil::Memset(dest, ' ', SFN_LEN);

    while (isPathSep(*srcName)) {
        srcName++;
    }

    extDot = srcName;
    p = srcName;
    nameLen = 0;
    while ((*p != '\0') && !isPathSep(*p)) {
        if (*p == '.') {
            extDot = p;
        }
        p++;
        nameLen++;
    }

    extExisted = (extDot > srcName) && (extDot < (srcName + nameLen - 1));

    p = srcName;
    for (int i = 0; (i < SFN_LEN) && (*p != '\0') && !isPathSep(*p); i++) {
        if (extExisted) {
            if (p == extDot) {
                dest = destName + 8;
                p++;
                i--;
                continue;
            } else if (p < extDot) {
                *dest++ = KernelUtil::ToUpper(*p++);
            } else {
                *dest++ = KernelUtil::ToUpper(*p++);
            }
        } else {
            *dest++ = KernelUtil::ToUpper(*p++);
        }
    }
}

u8 Fat32::getSfnCaseCfg(const char *sfnName) {
    u8 caseCfg = DIR_ITEM_NTRES_ALL_UPPER;
    const char *srcName = sfnName;
    const char *extDot;
    const char *p;
    int nameLen;
    int extExisted;

    while (isPathSep(*srcName)) {
        srcName++;
    }

    extDot = srcName;
    p = srcName;
    nameLen = 0;
    while ((*p != '\0') && !isPathSep(*p)) {
        if (*p == '.') {
            extDot = p;
        }
        p++;
        nameLen++;
    }

    extExisted = (extDot > srcName) && (extDot < (srcName + nameLen - 1));

    for (p = srcName; p < srcName + nameLen; p++) {
        if (extExisted) {
            if (p < extDot) {
                caseCfg |= KernelUtil::IsLower(*p) ? DIR_ITEM_NTRES_BODY_LOWER : 0;
            } else if (p > extDot) {
                caseCfg |= KernelUtil::IsLower(*p) ? DIR_ITEM_NTRES_EXT_LOWER : 0;
            }
        } else {
            caseCfg |= KernelUtil::IsLower(*p) ? DIR_ITEM_NTRES_BODY_LOWER : 0;
        }
    }

    return caseCfg;
}

bool Fat32::isFileNameMatch(const char *nameInDir, const char *toFindName) {
    char tempName[SFN_LEN];
    toSfn(tempName, toFindName);
    return KernelUtil::Memcmp(tempName, nameInDir, SFN_LEN) == 0;
}

const char *Fat32::skipFirstPathSep(const char *path) {
    const char *c = path;

    if (c == nullptr) {
        return nullptr;
    }

    while (isPathSep(*c)) {
        c++;
    }
    return c;
}

const char *Fat32::getChildPath(const char *dirPath) {
    const char *c = skipFirstPathSep(dirPath);

    while ((*c != '\0') && !isPathSep(*c)) {
        c++;
    }

    return (*c == '\0') ? nullptr : c + 1;
}

FileType Fat32::getFileTypeFromEntry(const DirEntry *entry) {
    if (entry->dirAttr & DIR_ITEM_ATTR_VOLUME_ID) {
        return FileType::Vol;
    } else if (entry->dirAttr & DIR_ITEM_ATTR_DIRECTORY) {
        return FileType::Dir;
    } else {
        return FileType::File;
    }
}

bool Fat32::Probe(Disk *disk) {
    if (disk == nullptr) {
        return false;
    }

    u8 block[512];
    disk->Read(0, (char *)block);
    u16 bytesPerSector = readLe16(block + SUPER_BLOCK_BYTES_PER_SECTOR_IDX);
    u8 sectorsPerCluster = block[SUPER_BLOCK_SECTOR_PER_CLUSTER_IDX];
    u16 reservedSectors = readLe16(block + SUPER_BLOCK_RESERVED_SECTOR_IDX);
    u8 fatCount = block[SUPER_BLOCK_FAT_NUM_IDX];
    u32 sectorsPerFat = readLe32(block + SUPER_BLOCK_SECTOR_PER_FAT_IDX);
    u32 root = readLe32(block + SUPER_BLOCK_ROOT_CLUSTER_IDX);

    u32 maxFatSectors = (4096u << (PAGE_GROUP_SIZE_BIT - 1)) / 512;
    if (block[510] != 0x55 || block[511] != 0xAA ||
        bytesPerSector != 512 || sectorsPerCluster == 0 ||
        sectorsPerCluster > 128 ||
        (sectorsPerCluster & (sectorsPerCluster - 1)) != 0 ||
        reservedSectors == 0 || fatCount == 0 || fatCount > 2 ||
        sectorsPerFat == 0 || sectorsPerFat > maxFatSectors ||
        readLe16(block + 17) != 0 || readLe16(block + 22) != 0 ||
        root < 2 || root >= sectorsPerFat * 128) {
        return false;
    }

    u32 totalSectors = readLe16(block + 19);
    if (totalSectors == 0) {
        totalSectors = readLe32(block + 32);
    }
    u64 dataStart = (u64)reservedSectors + (u64)fatCount * sectorsPerFat;
    if (totalSectors <= dataStart) {
        return false;
    }

    return KernelUtil::Memcmp(block + 82, "FAT32   ", 8) == 0 ||
           (totalSectors - dataStart) / sectorsPerCluster >= 65525;
}

void Fat32::Mount() {
    if (fatTable != nullptr || !Probe(disk)) {
        return;
    }

    char buf[512];
    KernelUtil::Memset(buf, 0, 512);
    disk->Read(0, buf);

    superBlock.bytesPerSector = readLe16((u8 *)buf + SUPER_BLOCK_BYTES_PER_SECTOR_IDX);
    superBlock.sectorPerCluster = (u8)buf[SUPER_BLOCK_SECTOR_PER_CLUSTER_IDX];
    superBlock.reservedSector = readLe16((u8 *)buf + SUPER_BLOCK_RESERVED_SECTOR_IDX);
    superBlock.fatNum = (u8)buf[SUPER_BLOCK_FAT_NUM_IDX];
    superBlock.sectorPerFat = readLe32((u8 *)buf + SUPER_BLOCK_SECTOR_PER_FAT_IDX);
    rootCluster = readLe32((u8 *)buf + SUPER_BLOCK_ROOT_CLUSTER_IDX);

    u8 level = 0;
    u32 sectors = 0x1000 / 0x200;
    while (sectors < superBlock.sectorPerFat) {
        ++level;
        sectors *= 2;
    }

    fatTable = (u32 *)pageAllocator.AllocPageMem(level);
    for (u32 i = 0; i < superBlock.sectorPerFat; i++) {
        disk->Read(superBlock.reservedSector + i, (char *)(fatTable + 512 / sizeof(u32) * i));
    }
}

u32 Fat32::getDirEntryCluster(DirEntry *entry) {
    return (entry->dirFstClusHI << 16) + entry->dirFstClusL0;
}

bool Fat32::isClusterValid(u32 cluster) {
    cluster &= 0x0FFFFFFF;
    return (cluster < 0x0FFFFFF0) && (cluster >= 0x2);
}

u32 Fat32::clusterFirstSector(u32 clusterNum) {
    u32 dataStartSector = superBlock.reservedSector + superBlock.fatNum * superBlock.sectorPerFat;
    return dataStartSector + (clusterNum - 2) * superBlock.sectorPerCluster;
}

u32 Fat32::getNextCluster(u32 curClusterNum) {
    return fatTable[curClusterNum];
}

void Fat32::readCluster(u8 *buf, u32 clusterNum) {
    int startSector = clusterFirstSector(clusterNum);
    for (int i = 0; i < superBlock.sectorPerCluster; i++) {
        disk->Read(startSector + i, (char *)(buf + 512 * i));
    }
}

void Fat32::writeCluster(u8 *buf, u32 clusterNum) {
    int startSector = clusterFirstSector(clusterNum);
    for (int i = 0; i < superBlock.sectorPerCluster; i++) {
        disk->Write(startSector + i, (char *)(buf + 512 * i));
    }
}

static char findBuf[512];

int Fat32::findEntry(u32 *parentCluster, u32 *parentClusterOffset,
                     const char *curPath, DirEntry *entryOut) {
    u32 currCluster = *parentCluster;

    int i = 0;
    char curPathBuf[13];
    char *searchPath;
    KernelUtil::Memset(curPathBuf, 0, 13);
    while (curPath[i] != '\0' && i <= 8) {
        if (i > 0 && curPath[i] == '.')
            break;
        i++;
    }
    if (i > 8) {
        KernelUtil::Memcpy(curPathBuf, curPath, 6);
        curPathBuf[6] = '~';
        curPathBuf[7] = '1';
        int dotpos = -1;
        for (int j = 8; curPath[j] != '\0'; j++) {
            if (curPath[j] == '.') {
                dotpos = j;
                break;
            }
        }
        if (dotpos != -1) {
            curPathBuf[8] = '.';
            KernelUtil::Memcpy(curPathBuf + 9, curPath + dotpos + 1, 3);
        }
        searchPath = curPathBuf;
    } else {
        searchPath = (char *)curPath;
    }

    do {
        u32 startSector = clusterFirstSector(currCluster);
        for (int si = 0; si < superBlock.sectorPerCluster; si++) {
            KernelUtil::Memset(findBuf, 0, 512);
            disk->Read(startSector + si, findBuf);
            for (int j = 0; j < 512 / sizeof(DirEntry); j++) {
                DirEntry *walkEntry = (DirEntry *)findBuf + j;
                if (walkEntry->dirName[0] == DIR_ITEM_NAME_END) {
                    return (int)FsError::Eof;
                }
                if (walkEntry->dirName[0] == DIR_ITEM_NAME_FREE) {
                    continue;
                }
                if (searchPath == nullptr || *searchPath == 0 ||
                    isFileNameMatch((const char *)walkEntry->dirName, searchPath)) {
                    *parentCluster = currCluster;
                    *parentClusterOffset = si * 512 + j * sizeof(DirEntry);
                    KernelUtil::Memcpy(entryOut, walkEntry, sizeof(DirEntry));
                    return (int)FsError::Ok;
                }
            }
        }
        currCluster = getNextCluster(currCluster);
    } while (isClusterValid(currCluster));

    return (int)FsError::Eof;
}

int Fat32::openSubFile(u32 dirCluster, FileHandle *file, const char *path) {
    u32 parentCluster = dirCluster;
    u32 parentClusterOffset = 0;
    path = skipFirstPathSep(path);
    if (path != nullptr && *path != '\0') {
        DirEntry entry;
        u32 fileStartCluster = 0;
        const char *curPath = path;
        while (curPath != nullptr) {
            int res = findEntry(&parentCluster, &parentClusterOffset, curPath, &entry);
            if (res != (int)FsError::Ok)
                return res;
            curPath = getChildPath(curPath);
            if (curPath != nullptr) {
                parentCluster = getDirEntryCluster(&entry);
                parentClusterOffset = 0;
            } else {
                fileStartCluster = getDirEntryCluster(&entry);
            }
        }
        file->size = entry.dirFileSize;
        file->type = getFileTypeFromEntry(&entry);
        file->attr = entry.dirAttr;
        file->startCluster = fileStartCluster;
        file->currCluster = fileStartCluster;
        file->dirCluster = parentCluster;
        file->dirClusterOffset = parentClusterOffset;
    } else {
        file->size = 0;
        file->type = FileType::Dir;
        file->attr = 0;
        file->startCluster = parentCluster;
        file->currCluster = parentCluster;
        file->dirCluster = CLUSTER_INVALID;
        file->dirClusterOffset = 0;
    }
    file->pos = 0;
    return 0;
}

int Fat32::Open(const char *path, FileHandle *file) {
    if (file == nullptr) {
        return (int)FsError::Param;
    }
    if (fatTable == nullptr) {
        return (int)FsError::FsType;
    }

    if (!isPathEnd(path)) {
        path = skipFirstPathSep(path);

        if (KernelUtil::Memcmp(path, "..", 2) == 0) {
            return -1;
        } else if (KernelUtil::Memcmp(path, ".", 1) == 0) {
            path++;
        }
    }
    return openSubFile(rootCluster, file, path);
}

static u8 readBuf[MAX_READ_BUF];

int Fat32::Read(FileHandle *file, u8 *buf, u64 count) {
    if (file == nullptr || buf == nullptr || fatTable == nullptr) {
        return (int)FsError::Param;
    }
    if (file->pos >= file->size || count == 0) {
        return 0;
    }
    if (count > file->size - file->pos) {
        count = file->size - file->pos;
    }

    u32 clusterBytes = superBlock.sectorPerCluster * 512;
    u32 clusterNum = file->startCluster;
    u32 clusterSkip = file->pos / clusterBytes;
    u32 offsetInCluster = file->pos % clusterBytes;
    for (u32 i = 0; i < clusterSkip && isClusterValid(clusterNum); ++i) {
        clusterNum = getNextCluster(clusterNum);
    }

    u64 bytesRead = 0;
    u8 sector[512];
    while (bytesRead < count && isClusterValid(clusterNum)) {
        u32 sectorInCluster = offsetInCluster / 512;
        u32 offsetInSector = offsetInCluster % 512;
        disk->Read(clusterFirstSector(clusterNum) + sectorInCluster,
                   (char *)sector);

        u32 available = 512 - offsetInSector;
        u64 left = count - bytesRead;
        u32 copySize = left < available ? (u32)left : available;
        KernelUtil::Memcpy(buf + bytesRead, sector + offsetInSector, copySize);
        bytesRead += copySize;
        offsetInCluster += copySize;

        if (offsetInCluster == clusterBytes) {
            clusterNum = getNextCluster(clusterNum);
            offsetInCluster = 0;
        }
    }

    file->pos += (u32)bytesRead;
    file->currCluster = clusterNum;
    return (int)bytesRead;
}

static u32 getEmptyCluster(Fat32 *fs, u32 currentCluster) {
    (void)currentCluster;
    (void)fs;
    return 0;
}

int Fat32::Write(FileHandle *file, u8 *buf, u64 count) {
    if (file->pos > file->size) {
        return 0;
    }
    KernelUtil::Memset(readBuf, 0, MAX_READ_BUF);
    if (file->pos + count > file->size)
        file->size += file->pos;
    if (file->pos + count > file->size) {
        file->size = file->pos + count;
    }
    u32 clusterCount = file->size / (superBlock.sectorPerCluster * 512);
    u32 clusterNum = file->startCluster;
    if (file->size % (superBlock.sectorPerCluster * 512) != 0)
        clusterCount += 1;

    for (int i = 0; i < clusterCount; i++) {
        if (!isClusterValid(clusterNum)) {
            clusterNum = getEmptyCluster(this, clusterNum);
            if (i == 0)
                file->startCluster = clusterNum;
        }
        readCluster(readBuf + i * superBlock.sectorPerCluster * 512, clusterNum);
        clusterNum = fatTable[clusterNum];
    }
    KernelUtil::Memcpy(readBuf + file->pos, buf, count);
    for (int i = 0, cn = file->startCluster; i < clusterCount; i++) {
        writeCluster(readBuf + i * superBlock.sectorPerCluster * 512, cn);
        cn = fatTable[cn];
    }
    for (int i = 0; i < superBlock.sectorPerFat; i++)
        disk->Write(superBlock.reservedSector + i, (char *)(fatTable + 512 / sizeof(u32) * i));

    KernelUtil::Memset(readBuf, 0, MAX_READ_BUF);
    readCluster(readBuf, file->dirCluster);
    DirEntry *entry = (DirEntry *)(readBuf + file->dirClusterOffset);
    entry->dirFileSize = file->size;
    entry->dirFstClusL0 = file->startCluster & 0x0000FFFF;
    entry->dirFstClusHI = file->startCluster >> 16;
    writeCluster(readBuf, file->dirCluster);
    return count;
}
