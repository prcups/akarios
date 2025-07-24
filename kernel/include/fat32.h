#ifndef FAT32_H_INCLUDED
#define FAT32_H_INCLUDED

#include <util.h>
#include <string.h>
#include <sdcard.h>
#include <uart.h>
#include <mem.h>

struct superBlock{
    unsigned short bytesPerSector;
    unsigned char sectorPerCluster;
    unsigned short reserverSector;
    unsigned char  fatNum;
    unsigned int sectorPerFat;
};

#define bytesPerSectorInex 0XB
#define sectorPerClusterIndex 0xD
#define reserverSectorIndex 0xE
#define fatNumIndex 0x10
#define sectorPerFatIndex 0x24
#define rootClusterIndex 0x2c

#define CLUSTER_INVALID                 0x0FFFFFFF          // 无效的簇号
#define CLUSTER_FREE                    0x00                // 空闲的cluster
#define FILE_DEFAULT_CLUSTER            0x00                // 文件的缺省簇号

#define DIRITEM_NAME_FREE               0xE5                // 目录项空闲名标记
#define DIRITEM_NAME_END                0x00                // 目录项结束名标记

#define DIRITEM_NTRES_BODY_LOWER        0x08                // 文件名小写
#define DIRITEM_NTRES_EXT_LOWER         0x10                // 扩展名小写
#define DIRITEM_NTRES_ALL_UPPER         0x00                // 文件名全部大写
#define DIRITEM_NTRES_CASE_MASK         0x18                // 大小写掩码

#define DIRITEM_ATTR_READ_ONLY          0x01                // 目录项属性：只读
#define DIRITEM_ATTR_HIDDEN             0x02                // 目录项属性：隐藏
#define DIRITEM_ATTR_SYSTEM             0x04                // 目录项属性：系统类型
#define DIRITEM_ATTR_VOLUME_ID          0x08                // 目录项属性：卷id
#define DIRITEM_ATTR_DIRECTORY          0x10                // 目录项属性：目录
#define DIRITEM_ATTR_ARCHIVE            0x20                // 目录项属性：归档
#define DIRITEM_ATTR_LONG_NAME          0x0F                // 目录项属性：长文件名

#define DIRITEM_GET_FREE        (1 << 0)
#define DIRITEM_GET_USED        (1 << 2)
#define DIRITEM_GET_END         (1 << 3)
#define DIRITEM_GET_ALL         0xff
typedef enum _xfile_type_t {
    FAT_DIR,
    FAT_FILE,
    FAT_VOL,
} xfile_type_t;
typedef enum _xfat_err_t {
    FS_ERR_EOF = 1,
    FS_ERR_OK = 0,
    FS_ERR_IO = -1,
    FS_ERR_PARAM = -2,
    FS_ERR_NONE = -3,
    FS_ERR_FSTYPE = -4,
}xfat_err_t;
struct  file{
    int count;
    unsigned int size;                     // 文件大小
    unsigned short attr;                     // 文件属性
    xfile_type_t type;              // 文件类型
    unsigned int pos;                      // 当前位置

    unsigned int start_cluster;            // 数据区起始簇号
    unsigned int curr_cluster;             // 当前簇号
    unsigned int dir_cluster;              // 所在的根目录的描述项起始簇号
    unsigned int dir_cluster_offset;       // 所在的根目录的描述项的簇偏移
};
#define OPENFILENUM 64
typedef struct _diritem_date_t {
    u16 day : 5;                  // 日
    u16 month : 4;                // 月
    u16 year_from_1980 : 7;       // 年
} diritem_date_t;

/**
 * FAT目录项的时间类型
 */
typedef struct _diritem_time_t {
    u16 second_2 : 5;             // 2秒
    u16 minute : 6;               // 分
    u16 hour : 5;                 // 时
} diritem_time_t;
struct dirEntry{
    u8 DIR_Name[8];                   // 文件名
    u8 DIR_ExtName[3];                // 扩展名
    u8 DIR_Attr;                      // 属性
    u8 DIR_NTRes;
    u8 DIR_CrtTimeTeenth;             // 创建时间的毫秒
    diritem_time_t DIR_CrtTime;         // 创建时间
    diritem_date_t DIR_CrtDate;         // 创建日期
    diritem_date_t DIR_LastAccDate;     // 最后访问日期
    u16 DIR_FstClusHI;                // 簇号高16位
    diritem_time_t DIR_WrtTime;         // 修改时间
    diritem_date_t DIR_WrtDate;         // 修改时期
    u16 DIR_FstClusL0;                // 簇号低16位
    u32 DIR_FileSize;
};
//u32 fatTable[];
void fat32_mount();
void fstest0();
#define is_path_sep(ch)         (((ch) == '\\') || ((ch == '/')))       // 判断是否是文件名分隔符
#define is_path_end(path)       (((path) == 0) || (*path == '\0'))  
#define SFN_LEN                     11   

int open(const char *path,struct file *file0);
int read(struct file *file0, unsigned char *bufDst, int count);
int write(struct file *file0, unsigned char *bufSrc, int count);

#endif // FAT32_H_INCLUDED
