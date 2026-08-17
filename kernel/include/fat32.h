#ifndef FAT32_H_INCLUDED
#define FAT32_H_INCLUDED

#include <filesystem.h>

#define SUPER_BLOCK_BYTES_PER_SECTOR_IDX      0x0B
#define SUPER_BLOCK_SECTOR_PER_CLUSTER_IDX    0x0D
#define SUPER_BLOCK_RESERVED_SECTOR_IDX       0x0E
#define SUPER_BLOCK_FAT_NUM_IDX               0x10
#define SUPER_BLOCK_SECTOR_PER_FAT_IDX        0x24
#define SUPER_BLOCK_ROOT_CLUSTER_IDX          0x2C

#define END_CLUSTER     0x0FFFFFFF
#define MAX_READ_BUF    (4 * 1024 * 1024)

struct SuperBlock {
    u16 bytesPerSector;
    u8  sectorPerCluster;
    u16 reservedSector;
    u8  fatNum;
    u32 sectorPerFat;
};

class Fat32 : public FileSystem {
    SuperBlock superBlock;
    u32 rootCluster;
    u32 *fatTable;

    u32 getDirEntryCluster(DirEntry *entry);
    bool isClusterValid(u32 cluster);
    u32 clusterFirstSector(u32 clusterNum);
    u32 getNextCluster(u32 curClusterNum);
    void readCluster(u8 *buf, u32 clusterNum);
    void writeCluster(u8 *buf, u32 clusterNum);

    FileType getFileTypeFromEntry(const DirEntry *entry);
    void toSfn(char *destName, const char *srcName);
    u8 getSfnCaseCfg(const char *sfnName);
    bool isFileNameMatch(const char *nameInDir, const char *toFindName);
    const char *skipFirstPathSep(const char *path);
    const char *getChildPath(const char *dirPath);

    int findEntry(u32 *parentCluster, u32 *parentClusterOffset,
                  const char *curPath, DirEntry *entryOut);
    int openSubFile(u32 dirCluster, FileHandle *file, const char *path);

public:
    Fat32(Disk *d) : FileSystem(d), fatTable(nullptr) {}

    static bool Probe(Disk *disk);
    void Mount() override;
    int Open(const char *path, FileHandle *file) override;
    int Read(FileHandle *file, u8 *buf, u64 count) override;
    int Write(FileHandle *file, u8 *buf, u64 count) override;
};

#endif // FAT32_H_INCLUDED
