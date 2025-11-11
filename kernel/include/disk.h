#ifndef DISK_H_INCLUDED
#define DISK_H_INCLUDED

#include <util.h>
#include <list.h>

class Disk {
public:
    virtual void Read(u64 blockNum, char *buf) = 0;
    virtual void Write(u64 blockNum, char *buf) = 0;
};

extern ListItem<Disk*> *diskList;

#endif // DISK_H_INCLUDED
