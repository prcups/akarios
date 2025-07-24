#include <process.h>
#include <string.h>
u64 FileTable::Open(const char *filePath) {
    //open(filePath, &sdFile[++processFd]);
    return processFd;
}

void FileTable::Close(u64 fd)
{
    sdFile[fd].size = 0;
}

int FileTable::Read(u64 fd, u8* buf, u64 size)
{
    /*if (sdFile[fd].size != 0) {
        return read(&sdFile[fd], buf, size);
    } else*/
        return -1;
}

int FileTable::Write(u64 fd, u8* buf, u64 size) {
    // if (sdFile[fd].size != 0) {
    //     return write(&sdFile[fd], buf, size);
    // } else
        return -1;
}

int FileTable::Getcwd(u8* buf, u64 size) {
    KernelUtil::Memcpy(buf,processController.CurrentProcess->pwd,128);
    return 100;
}

int FileTable::Chdir(u8* buf) {
    KernelUtil::Memcpy(processController.CurrentProcess->pwd,buf,sizeof(buf));
    return 0;
}
