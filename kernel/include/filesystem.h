#ifndef FILESYSTEM_H_INCLUDED
#define FILESYSTEM_H_INCLUDED

#include <util.h>
#include <disk.h>

#define SFN_LEN                     11
#define OPEN_FILE_NUM               64

#define CLUSTER_INVALID             0x0FFFFFFF
#define CLUSTER_FREE                0x00
#define FILE_DEFAULT_CLUSTER        0x00

#define DIR_ITEM_NAME_FREE          0xE5
#define DIR_ITEM_NAME_END           0x00

#define DIR_ITEM_NTRES_BODY_LOWER   0x08
#define DIR_ITEM_NTRES_EXT_LOWER    0x10
#define DIR_ITEM_NTRES_ALL_UPPER    0x00
#define DIR_ITEM_NTRES_CASE_MASK    0x18

#define DIR_ITEM_ATTR_READ_ONLY     0x01
#define DIR_ITEM_ATTR_HIDDEN        0x02
#define DIR_ITEM_ATTR_SYSTEM        0x04
#define DIR_ITEM_ATTR_VOLUME_ID     0x08
#define DIR_ITEM_ATTR_DIRECTORY     0x10
#define DIR_ITEM_ATTR_ARCHIVE       0x20
#define DIR_ITEM_ATTR_LONG_NAME     0x0F

#define DIR_ITEM_GET_FREE           (1 << 0)
#define DIR_ITEM_GET_USED           (1 << 2)
#define DIR_ITEM_GET_END            (1 << 3)
#define DIR_ITEM_GET_ALL            0xFF

enum class FileType {
    Dir,
    File,
    Vol,
};

enum class FsError {
    Eof = 1,
    Ok = 0,
    Io = -1,
    Param = -2,
    None = -3,
    FsType = -4,
};

struct DirItemDate {
    u16 day : 5;
    u16 month : 4;
    u16 yearFrom1980 : 7;
};

struct DirItemTime {
    u16 second2 : 5;
    u16 minute : 6;
    u16 hour : 5;
};

struct DirEntry {
    u8 dirName[8];
    u8 dirExtName[3];
    u8 dirAttr;
    u8 dirNTRes;
    u8 dirCrtTimeTeenth;
    DirItemTime dirCrtTime;
    DirItemDate dirCrtDate;
    DirItemDate dirLastAccDate;
    u16 dirFstClusHI;
    DirItemTime dirWrtTime;
    DirItemDate dirWrtDate;
    u16 dirFstClusL0;
    u32 dirFileSize;
};

struct FileHandle {
    u32 size;
    u16 attr;
    FileType type;
    u32 pos;
    u32 startCluster;
    u32 currCluster;
    u32 dirCluster;
    u32 dirClusterOffset;
};

class FileSystem {
protected:
    Disk *disk;
public:
    FileSystem(Disk *d) : disk(d) {}
    virtual ~FileSystem() {}

    virtual void Mount() = 0;
    virtual int Open(const char *path, FileHandle *file) = 0;
    virtual int Read(FileHandle *file, u8 *buf, u64 count) = 0;
    virtual int Write(FileHandle *file, u8 *buf, u64 count) = 0;
};

extern ListItem<FileSystem*> *fsList;

inline bool isPathSep(char ch) {
    return (ch == '\\') || (ch == '/');
}

inline bool isPathEnd(const char *path) {
    return (path == nullptr) || (*path == '\0');
}

#endif // FILESYSTEM_H_INCLUDED
