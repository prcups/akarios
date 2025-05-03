#include <efi.h>
#include <stddef.h>
#include <protocol/efi-sfsp.h>
#include <protocol/efi-lip.h>

#define PAGE_SIZE     4096
#define KERNEL_ADDR   0xA00000
#define KERNEL_PATH   "\\kernel.bin"

typedef struct
{
  UINT8                   *RsdpPtr;
  UINTN                   MemMapSize;
  EFI_MEMORY_DESCRIPTOR   *MemMap;
} BootInfo;

INTN CompareGuid (EFI_GUID *Guid1, EFI_GUID *Guid2) {
  INT32       *g1, *g2, r;

  g1 = (INT32 *) Guid1;
  g2 = (INT32 *) Guid2;

  r  = g1[0] - g2[0];
  r |= g1[1] - g2[1];
  r |= g1[2] - g2[2];
  r |= g1[3] - g2[3];

  return r;
}

EFI_STATUS EFIAPI efiMain (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS                      status;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *simpleFileSystem;
  EFI_FILE_PROTOCOL               *root, *kernel, *handle;
  EFI_LOADED_IMAGE_PROTOCOL       *loadedImageProtocol;

  UINTN                           kernelSize = 0xffffffff;
  VOID                            *kernelMain;
  EFI_PHYSICAL_ADDRESS            buffer = (EFI_PHYSICAL_ADDRESS)KERNEL_ADDR;

  UINTN                           memMapSize = 0;
  EFI_MEMORY_DESCRIPTOR           *memMap = NULL;
  UINTN                           mapKey;
  UINTN                           despSize;
  UINT32                          despVer;

  EFI_GUID                        acpiTableGuid = EFI_ACPI_20_TABLE_GUID;
  EFI_GUID                        simpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_GUID                        loadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_CONFIGURATION_TABLE         *configurationTable;
  UINT8                           *rsdpPtr = NULL;
  UINT8                           rsdpRevision;
  UINTN                           index;

  BootInfo                        *bootInfo = (BootInfo *)(KERNEL_ADDR - sizeof(BootInfo));

  for (index = 0; index < SystemTable->NumberOfTableEntries; index++)
  {
    if (&SystemTable->ConfigurationTable[index].VendorGuid == &acpiTableGuid)
    {
      configurationTable = &SystemTable->ConfigurationTable[index];
      rsdpPtr = (UINT8 *)configurationTable->VendorTable;
      break;
    }
  }

  SystemTable->BootServices->LocateProtocol(&simpleFileSystemProtocolGuid, NULL, (void **)&simpleFileSystem);
  status = simpleFileSystem->OpenVolume(simpleFileSystem, &root);
  status = SystemTable->BootServices->HandleProtocol(ImageHandle, &loadedImageProtocolGuid, (VOID **)&loadedImageProtocol);
  status = SystemTable->BootServices->HandleProtocol(loadedImageProtocol->DeviceHandle, &simpleFileSystemProtocolGuid, (VOID **)&simpleFileSystem);

  status = simpleFileSystem->OpenVolume(simpleFileSystem, &kernel);

  status = kernel->Open(kernel, &handle, KERNEL_PATH, EFI_FILE_MODE_READ, 0);

  status = handle->SetPosition(handle, kernelSize);
  status = handle->GetPosition(handle, &kernelSize);
  status = handle->SetPosition(handle, 0);

  status = SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, ((kernelSize + PAGE_SIZE) / PAGE_SIZE), &buffer);

  status = handle->Read(handle, &kernelSize, (VOID *)buffer);
  status = handle->Close(handle);

  status = root->Close(root);

  kernelMain = (VOID*) buffer;

  status = SystemTable->BootServices->GetMemoryMap(&memMapSize, (EFI_MEMORY_DESCRIPTOR *)memMap, &mapKey, &despSize, &despVer);
  memMapSize += 1024 * despSize;
  status = SystemTable->BootServices->AllocatePool(EfiLoaderData, memMapSize, (void **)&memMap);
  status = SystemTable->BootServices->GetMemoryMap(&memMapSize, (EFI_MEMORY_DESCRIPTOR *)memMap, &mapKey, &despSize, &despVer);
  status = SystemTable->BootServices->ExitBootServices(ImageHandle, mapKey);

  bootInfo->RsdpPtr = rsdpPtr;
  bootInfo->MemMap = memMap;
  bootInfo->MemMapSize = memMapSize;

  // __asm__ volatile (
  //     "ld.d $sp, %0\n"
  //     "ld.d $t0, %1\n"
  //     "jr $t0\n"
  //     ::"m"(bootInfo), "m"(kernelMain)
  //     :"$sp", "$t0");

  return EFI_SUCCESS;
}
