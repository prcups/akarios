#include <fat32.h>
#include <string.h>
#include <sdcard.h>
#include <util.h>
#include <uart.h>
#include <mem.h>
struct file fileTable[OPENFILENUM];
unsigned int rootCluster;
struct superBlock SUPERBLOCK;
unsigned int *fatTable;
#define printd(str0) //uPut<<str0<<"\n"
static xfat_err_t to_sfn(char* dest_name, const char* my_name) {
    int i, name_len;
    char * dest = dest_name;
    const char * ext_dot;
    const char * p;
    int ext_existed;

    KernelUtil::Memset(dest, ' ', SFN_LEN);

    // 跳过开头的分隔符
    while (is_path_sep(*my_name)) {
        my_name++;
    }

    // 找到第一个斜杠之前的字符串，将ext_dot定位到那里，且记录有效长度
    ext_dot = my_name;
    p = my_name;
    name_len = 0;
    while ((*p != '\0') && !is_path_sep(*p)) {
        if (*p == '.') {
            ext_dot = p;
        }
        p++;
        name_len++;
    }

    // 如果文件名以.结尾，意思就是没有扩展名？
    // todo: 长文件名处理?
    ext_existed = (ext_dot > my_name) && (ext_dot < (my_name + name_len - 1));

    // 遍历名称，逐个复制字符, 算上.分隔符，最长12字节，如果分离符，则只应有
    p = my_name;
    for (i = 0; (i < SFN_LEN) && (*p != '\0') && !is_path_sep(*p); i++) {
        if (ext_existed) {
            if (p == ext_dot) {
                dest = dest_name + 8;
                p++;
                i--;
                continue;
            }
            else if (p < ext_dot) {
                *dest++ = KernelUtil::ToUpper(*p++);
            }
            else {
                *dest++ = KernelUtil::ToUpper(*p++);
            }
        }
        else {
            *dest++ = KernelUtil::ToUpper(*p++);
        }
    }
    return FS_ERR_OK;
}


/**
 * 检查sfn字符串中是否是大写。如果中间有任意小写，都认为是小写
 * @param name
 * @return
 */
static u8 get_sfn_case_cfg(const char * sfn_name) {
    u8 case_cfg = 0;

    int name_len;
    const char * src_name = sfn_name;
    const char * ext_dot;
    const char * p;
    int ext_existed;

    // 跳过开头的分隔符
    while (is_path_sep(*src_name)) {
        src_name++;
    }

    // 找到第一个斜杠之前的字符串，将ext_dot定位到那里，且记录有效长度
    ext_dot = src_name;
    p = src_name;
    name_len = 0;
    while ((*p != '\0') && !is_path_sep(*p)) {
        if (*p == '.') {
            ext_dot = p;
        }
        p++;
        name_len++;
    }

    // 如果文件名以.结尾，意思就是没有扩展名？
    // todo: 长文件名处理?
    ext_existed = (ext_dot > src_name) && (ext_dot < (src_name + name_len - 1));
    for (p = src_name; p < src_name + name_len; p++) {
        if (ext_existed) {
            if (p < ext_dot) { // 文件名主体部分大小写判断
                case_cfg |= KernelUtil::IsLower(*p) ? DIRITEM_NTRES_BODY_LOWER : 0;
            } else if (p > ext_dot) {
                case_cfg |= KernelUtil::IsLower(*p) ? DIRITEM_NTRES_EXT_LOWER : 0;
            }
        } else {
            case_cfg |= KernelUtil::IsLower(*p) ? DIRITEM_NTRES_BODY_LOWER : 0;
        }
    }

    return case_cfg;
}

/**
 * 判断两个文件名是否匹配
 * @param name_in_item fatdir中的文件名格式
 * @param my_name 应用可读的文件名格式
 * @return
 */
static u8 is_filename_match(const char *name_in_dir, const char *to_find_name) {
    char temp_name[SFN_LEN];

    // FAT文件名的比较检测等，全部转换成大写比较
    // 根据目录的大小写配置，将其转换成8+3名称，再进行逐字节比较
    // 但实际显示时，会根据diritem->NTRes进行大小写转换
    to_sfn(temp_name, to_find_name);
    return KernelUtil::Memcmp(temp_name, name_in_dir, SFN_LEN) == 0;
}
static const char * skip_first_path_sep (const char * path) {
    const char * c = path;

    if (c == (const char *)0) {
        return (const char *)0;
    }

    // 跳过开头的分隔符
    while (is_path_sep(*c)) {
        c++;
    }
    return c;
}

/**
 * 获取子路径
 * @param dir_path 上一级路径
 * @return
 */
const char * get_child_path(const char *dir_path) {
    const char * c = skip_first_path_sep(dir_path);

    // 跳过父目录
    while ((*c != '\0') && !is_path_sep(*c)) {
        c++;
    }

    return (*c == '\0') ? (const char *)0 : c + 1;
}
void printStr(u8 *str0){
    int i=0;
    while(str0[i]){
        uPut<<(char )str0[i];
        i++;
    }
    uPut<<"\n";
}

static xfile_type_t get_file_type(const dirEntry *diritem) {
    xfile_type_t type;

    if (diritem->DIR_Attr & DIRITEM_ATTR_VOLUME_ID) {
        type = FAT_VOL;
    } else if (diritem->DIR_Attr & DIRITEM_ATTR_DIRECTORY) {
        type = FAT_DIR;
    } else {
        type = FAT_FILE;
    }

    return type;
}
int clusterLevel = 0;
void fat32_mount(){
    char buf[512];
    KernelUtil::Memset(buf,0,512);
    sdcard.ReadBlock(0,buf);
    SUPERBLOCK.bytesPerSector = *(unsigned short *)(buf + bytesPerSectorInex);
    SUPERBLOCK.sectorPerCluster = *(unsigned char *)(buf + sectorPerClusterIndex);
    SUPERBLOCK.reserverSector = *(unsigned short *)(buf + reserverSectorIndex);
    SUPERBLOCK.fatNum = *(unsigned char *)(buf+fatNumIndex);
    SUPERBLOCK.sectorPerFat = *(unsigned int *)(buf + sectorPerFatIndex);
    //no complete, bug
    //SUPERBLOCK.fatClusterNum = SUPERBLOCK.sectorPerFat/SUPERBLOCK.sectorPerCluster;
    rootCluster = *(unsigned int *)(buf + rootClusterIndex);
    int level =  0;
    // cal fat size
    int sectors = 0x1000 / 0x200;  //8
    for(;sectors<=SUPERBLOCK.sectorPerFat; level++,sectors*=2)
        ;
    int pageSize = 0x1000;
    fatTable = (unsigned int *)pageAllocator.AllocPageMem(level);
    for(int i=0; i<SUPERBLOCK.sectorPerFat; i++)
        sdcard.ReadBlock((SUPERBLOCK.reserverSector+i)*512,(void *)(fatTable+512/sizeof(u32)*i));
    for(;pageSize >= SUPERBLOCK.sectorPerCluster*0x200; pageSize*=2)
        ;
    for(int i=0; i<OPENFILENUM; i++){
        fileTable[i].count = 0;
    }
    //uPut<<"secClus="<<SUPERBLOCK.sectorPerCluster*512<<"\n";
}


u32 getDirEntryCluster(struct dirEntry *dirEntry0){
    return (dirEntry0->DIR_FstClusHI << 16) + dirEntry0->DIR_FstClusL0;
}
int is_cluster_valid(u32 cluster) {
    cluster &= 0x0FFFFFFF;
    return (cluster < 0x0FFFFFF0) && (cluster >= 0x2);     // 值是否正确
}
u32 clusterFirstSector(u32 clusterNum){
    u32 dataStartSector = SUPERBLOCK.reserverSector + SUPERBLOCK.fatNum * SUPERBLOCK.sectorPerFat;
    u32 sectorNum = dataStartSector + (clusterNum-2) *SUPERBLOCK.sectorPerCluster;
    return sectorNum;
}
u32 getNextCluster(u32 curClusrerNum){
    return fatTable[curClusrerNum];
}
char bufPublic[512];
int findEntry(u32 *parentCluster, u32 *parentClusterOffet, const char *curPath, struct dirEntry *dirEntry0){
    u32 currCluster = *parentCluster;
    u32 initialSector = 0;

    const char *t = curPath;
    int i = 0;
    char curPath0[13];
    char *curPath1;
    KernelUtil::Memset(curPath0,0,13);
    while(t[i] !='\0' && i<=8){
        if(i>0 && t[i]=='.')
            break;
        i++;
    }
    //uPut<<"i="<<i<<"\n";
    if(i>8){
        KernelUtil::Memcpy(curPath0,curPath,6);
        curPath0[6] = '~', curPath0[7] = '1';
        int dotpos = -1;
        for(int i=8; curPath[i] != '\0'; i++){
            if(curPath[i] == '.'){
                dotpos = i;
                break;
            }
        }
        if(dotpos != -1){
            curPath0[8] = '.';
            KernelUtil::Memcpy(curPath0+9,curPath+dotpos+1,3);
        }
        curPath1 = curPath0;
    }
    else
        curPath1 = (char *)curPath;
    //uPut<<"path="<<curPath<<"\n";
    //printStr((u8 *)curPath1);

    do{
        u32 startSector = clusterFirstSector(currCluster);
        u32 i;
        for(i=0; i<SUPERBLOCK.sectorPerCluster; i++){
            u32 j;
            KernelUtil::Memset(bufPublic,0,512);
            ////uPut<<"--------------- i="<<i<<"\n";
            sdcard.ReadBlock((startSector+i)*512,bufPublic);
            for(j=0; j<512/sizeof(struct dirEntry); j++){
                struct dirEntry *walkEntry = (struct dirEntry *)bufPublic + j;
                //uPut<<"***************** i="<<i<<"    "<<"j="<<j<<"\n";
                if(walkEntry->DIR_Name[0] == DIRITEM_NAME_END){
                    //uPut<<"a i="<<i<<"    "<<"j="<<j<<"\n";
                    return FS_ERR_EOF;
                }
                if(walkEntry->DIR_Name[0] == DIRITEM_NAME_FREE){
                    //uPut<<"b i="<<i<<"    "<<"j="<<j<<"\n";
                    continue;
                }
                if(curPath1 == NULL || *curPath1 == 0 || is_filename_match((const char *)walkEntry->DIR_Name,curPath1)){
                    //uPut<<"c i="<<i<<"    "<<"j="<<j<<"\n";
                    *parentCluster = currCluster;
                    *parentClusterOffet = i*512+j*sizeof(struct dirEntry);
                    //uPut<<"walkEntry->size="<<walkEntry->DIR_FileSize<<"\n";
                    KernelUtil::Memcpy(dirEntry0,walkEntry,sizeof(struct dirEntry));
                    return FS_ERR_OK;
                }
            }
        }
        currCluster = getNextCluster(currCluster);

    }while(is_cluster_valid(currCluster));
    return FS_ERR_EOF;
}
void readCluster(u8 *bufClusrer,u32 clusterNum){
    int startSector = clusterFirstSector(clusterNum);
    //uPut<<"startsector="<<startSector<<"\n";
    for(int i=0; i<SUPERBLOCK.sectorPerCluster; i++){
        sdcard.ReadBlock((startSector+i)*512,(u8 *)bufClusrer+512*i);
    }
}
void writeCluster(u8 *bufClusrer,u32 clusterNum){
    int startSector = clusterFirstSector(clusterNum);
    for(int i=0; i<SUPERBLOCK.sectorPerCluster; i++){
        sdcard.WriteBlock((startSector+i)*512,(u8 *)bufClusrer+512*i);
    }
}
int openSubFile(u32 dirCluster, struct file *file0,const char *path){
    u32 parentCluster = dirCluster;
    u32 parentClusterOffset = 0;
    path = skip_first_path_sep(path);
    if((path != 0) && (*path != '\0')){
        struct dirEntry dirEntry0;
        u32 fileStartCluster = 0;
        const char *curPath = path;
        ////uPut<<"path="<<curPath<<"\n";
        ////uPut<<"parentCluster="<<(unsigned int *)parentCluster<<"\n";
        while(curPath != (const char *)0){
            int res = -1;
            res = findEntry(&parentCluster,&parentClusterOffset,curPath,&dirEntry0);
            if(res == -1)
                return -1;
            curPath = get_child_path(curPath);
            if(curPath != NULL){
                parentCluster = getDirEntryCluster(&dirEntry0);
                parentClusterOffset = 0;
            }else{
                fileStartCluster = getDirEntryCluster(&dirEntry0);
            }
        }
        //printd("****************************************");
        //uPut<<"dirEntry0.DIR_FileSize="<<dirEntry0.DIR_FileSize<<"\n";
        file0->size = dirEntry0.DIR_FileSize;
        file0->type = get_file_type(&dirEntry0);
        file0->attr = dirEntry0.DIR_Attr;
        file0->start_cluster = getDirEntryCluster(&dirEntry0);
        file0->curr_cluster = fileStartCluster;
        file0->dir_cluster = parentCluster;
        file0->dir_cluster_offset = parentClusterOffset;
    }else{
        file0->size = 0;
        file0->type = FAT_DIR;
        file0->attr = 0;
        file0->start_cluster = parentCluster;
        file0->curr_cluster = parentCluster;
        file0->dir_cluster = CLUSTER_INVALID;
        file0->dir_cluster_offset = 0;
    }
    file0->pos = 0;
    return 0;
}
int open(const char *path,struct file *file0){
    if (!is_path_end(path)) {
        path = skip_first_path_sep(path);

        // 根目录不存在上级目录
        // 若含有.，直接过滤掉路径
        if (KernelUtil::Memcmp(path, "..", 2) == 0) {
            return -1;
        } else if (KernelUtil::Memcmp(path, ".", 1) == 0) {
            path++;
        }
    }
    openSubFile(rootCluster,file0,path);
    return 0;
}

#define MAXLEN 4*1024*1024
u8 bufTemp[MAXLEN];
int read(struct file *file0, unsigned char *bufDst, int count){
    u32 alreadyReaded = 0;
    if(file0->pos > file0->size){
        return 0;
    }
    if(file0->pos + count > file0->size){
        count = file0->size - file0->pos; 
    }
    int filesize = file0->size,pageSize = 0x1000;
    int level = 0;
    for(;pageSize>=filesize; level++)
        pageSize*=2;
    //u8 *bufTemp = (u8 *)pageAllocator.AllocPageMem(level);
    int clusterCount = filesize / (SUPERBLOCK.sectorPerCluster*512);
    clusterCount = (filesize % (SUPERBLOCK.sectorPerCluster*512) == 0)? (clusterCount):(clusterCount+1);
    int cluterNum = file0->start_cluster;
    //uPut<<"clustercount="<<clusterCount<<"\n";
    KernelUtil::Memset(bufTemp,0,MAXLEN);
    for(int i=0; i<clusterCount && is_cluster_valid(cluterNum); i++){
        readCluster(bufTemp+i*SUPERBLOCK.sectorPerCluster*512,cluterNum);
        cluterNum = fatTable[cluterNum];
    }
    KernelUtil::Memcpy(bufDst,bufTemp+file0->pos,count);
    return count;
}
#define ENDCLUSTER 0x0FFFFFFF
int getEmptyCluster(u32 currentCluster){
    //uPut<<"currentCluster="<<(u32 *)currentCluster<<"\n";
    u32 i;
    for(i=0; i<SUPERBLOCK.sectorPerCluster*512/4; i++){
        if(fatTable[i] == 0)
            break;
    }
    fatTable[currentCluster] = i;
    fatTable[i] = ENDCLUSTER;
    return i;
}
int write(struct file *file0, unsigned char *bufSrc, int count){
    if(file0->pos > file0->size){
        return 0;
    }
    KernelUtil::Memset(bufTemp,0,MAXLEN);
    if(file0->pos + count > file0->size)
        file0->size += file0->pos; 
    if(file0->pos + count > file0->size){
        file0->size = file0->pos +count;
    }
    u32 clusterCount = file0->size / (SUPERBLOCK.sectorPerCluster*512);
    u32 clusterNum = file0->start_cluster;
    if( file0->size % (SUPERBLOCK.sectorPerCluster*512) != 0)
        clusterCount += 1;
    //uPut<<"clustercount="<<clusterCount<<"\n";
    for(int i=0; i<clusterCount; i++){
        //uPut<<"clusterNum ="<<(u32 *)clusterNum << "\n";
        if(!is_cluster_valid(clusterNum)){
            //uPut<<"-----------------------------------------------------------------"<<"\n";
            clusterNum =getEmptyCluster(clusterNum);
            if(i==0)
                file0->start_cluster = clusterNum;
        }
        //uPut<<"clusterNum ="<<(u32 *)clusterNum << "\n";
        readCluster(bufTemp+i*SUPERBLOCK.sectorPerCluster*512,clusterNum);
        clusterNum = fatTable[clusterNum];
    }
    KernelUtil::Memcpy(bufTemp+file0->pos,bufSrc,count);
    for(int i=0,clusterNum = file0->start_cluster; i<clusterCount; i++){
        writeCluster(bufTemp+i*SUPERBLOCK.sectorPerCluster*512,clusterNum);
        clusterNum = fatTable[clusterNum];
    }
    for(int i=0; i<SUPERBLOCK.sectorPerFat; i++)
        sdcard.WriteBlock((SUPERBLOCK.reserverSector+i)*512,(void *)(fatTable+512/sizeof(u32)*i));
    KernelUtil::Memset(bufTemp,0,MAXLEN);
    readCluster(bufTemp,file0->dir_cluster);
    struct dirEntry* dirEntry0 = (struct dirEntry*)((u8 *)bufTemp+file0->dir_cluster_offset);
    //uPut<<"file0->dir_cluster="<<file0->dir_cluster<<" file0->dir_cluster_offset="<<file0->dir_cluster_offset<<"\n";
    //printStr(dirEntry0->DIR_Name);
    dirEntry0->DIR_FileSize = file0->size;
    dirEntry0->DIR_FstClusL0 = file0->start_cluster & 0x0000FFFF;
    dirEntry0->DIR_FstClusHI = file0->start_cluster >> 16;
    writeCluster(bufTemp,file0->dir_cluster);
    //uPut<<"clucount="<<clusterCount<<"  "<<"file0->start_cluster = "<<file0->start_cluster<<"\n";
    return count;
}
// xfat_err_t createSubFile(u8 isDir, u32 parentCluster,const char *childName, u32 fileCluster){
//     while(1){
//         struct dirEntry *entry0 = NULL;
        
//     }
// }
// xfat_err_t createEmptyDir(u32 parentCluster, u32 *newCluster, const char *name){

// }
// int create(const char *path,struct file *file0){

// }
u8 buf1[4096*4];
void fstest0(){
    struct file file0;
    KernelUtil::Memset(&file0,0,sizeof(struct file));
    //uPut<<file0.size<<"\n";
    //uPut<<"start0"<<"\n";
    open("/wrt1",&file0);
    //uPut<<"finish0"<<"\n";
    //uPut<<file0.size<<"\n";
    KernelUtil::Memset(buf1,0,4096*4);
    read(&file0,buf1,15000);
    printStr(buf1);
    //KernelUtil::Memcpy(buf1,"test write666",sizeof("test write666"));
    //write(&file0,buf1,sizeof("test write666"));

}
