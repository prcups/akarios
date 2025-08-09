#ifndef ACPI_H_INCLUDED
#define ACPI_H_INCLUDED

#include <util.h>
#include <uart.h>

struct ACPISDTHeader
{
    char Signature[4];
    u32 Length;
    u8 Revision;
    u8 Checksum;
    char OEMID[6];
    char OEMTableID[8];
    u32 OEMRevision;
    u32 CreatorID;
    u32 CreatorRevision;
};

struct XSDP
{
    char Signature[8];
    u8 Checksum;
    char OEMID[6];
    u8 Revision;
    u32 RsdtAddress;
    u32 Length;
    ACPISDTHeader *XsdtAddress;
    u8 ExtendedChecksum;
    u8 reserved[3];
};

class ACPIManager
{
    ACPISDTHeader *root, **table;
public:
    void Init(XSDP* xsdp);
    ACPISDTHeader* FindTable(const char* name);
};

#endif
