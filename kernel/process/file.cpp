#include <process.h>
#include <string.h>

u64 FileTable::Open(const char *filePath) {
    if (fs == nullptr) return -1;
    FileHandle *handle = &fileTable[++processFd];
    KernelUtil::Memset(handle, 0, sizeof(FileHandle));
    fs->Open(filePath, handle);
    return processFd;
}

void FileTable::Close(u64 fd) {
    fileTable[fd].size = 0;
}

int FileTable::Read(u64 fd, u8* buf, u64 size) {
    if (fs == nullptr) return -1;
    if (fileTable[fd].size != 0) {
        return fs->Read(&fileTable[fd], buf, size);
    }
    return -1;
}

int FileTable::Write(u64 fd, u8* buf, u64 size) {
    if (fs == nullptr) return -1;
    if (fileTable[fd].size != 0) {
        return fs->Write(&fileTable[fd], buf, size);
    }
    return -1;
}

int FileTable::Getcwd(u8* buf, u64 size) {
    KernelUtil::Memcpy(buf, processController.CurrentProcess->pwd, 128);
    return 100;
}

int FileTable::Chdir(u8* buf) {
    KernelUtil::Memcpy(processController.CurrentProcess->pwd, buf, sizeof(buf));
    return 0;
}
