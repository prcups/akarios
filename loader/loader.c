#include <efi.h>
#include <efilib.h>

#define PAGE_SIZE     4096
#define KERNEL_ADDR   0xA00000
#define KERNEL_PATH   L"\\kernel.bin"

typedef struct {
  UINT8* RsdpPtr;
  EFI_MEMORY_DESCRIPTOR* MemMap;
  UINT32 DespSize;
  UINT32 MemMapSize;
} BootInfo;

EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  InitializeLib(ImageHandle, SystemTable);

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
  EFI_GUID                        acpiTableGuid = ACPI_20_TABLE_GUID;
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

  uefi_call_wrapper(SystemTable->BootServices->LocateProtocol, 3, &simpleFileSystemProtocolGuid, NULL, (void **)&simpleFileSystem);
  uefi_call_wrapper(simpleFileSystem->OpenVolume, 2, simpleFileSystem, &root);
  uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, ImageHandle, &loadedImageProtocolGuid, (VOID **)&loadedImageProtocol);
  uefi_call_wrapper(SystemTable->BootServices->HandleProtocol, 3, loadedImageProtocol->DeviceHandle, &simpleFileSystemProtocolGuid, (VOID **)&simpleFileSystem);
  uefi_call_wrapper(simpleFileSystem->OpenVolume, 2, simpleFileSystem, &kernel);
  uefi_call_wrapper(kernel->Open, 5, kernel, &handle, KERNEL_PATH, EFI_FILE_MODE_READ, 0);
  uefi_call_wrapper(handle->SetPosition, 2, handle, kernelSize);
  uefi_call_wrapper(handle->GetPosition, 2, handle, &kernelSize);
  uefi_call_wrapper(handle->SetPosition, 2, handle, 0);
  uefi_call_wrapper(SystemTable->BootServices->AllocatePages, 4, AllocateAddress, EfiLoaderData, ((kernelSize + PAGE_SIZE) / PAGE_SIZE), &buffer);
  uefi_call_wrapper(handle->Read, 3, handle, &kernelSize, (VOID *)buffer);
  uefi_call_wrapper(handle->Close, 1, handle);
  uefi_call_wrapper(root->Close, 1, root);
  kernelMain = (VOID*) buffer;

  uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap, 5, &memMapSize, (EFI_MEMORY_DESCRIPTOR *)memMap, &mapKey, &despSize, &despVer);
  memMapSize += 1024 * despSize;
  uefi_call_wrapper(SystemTable->BootServices->AllocatePool, 3, EfiLoaderData, memMapSize, (void **)&memMap);
  uefi_call_wrapper(SystemTable->BootServices->GetMemoryMap, 5, &memMapSize, (EFI_MEMORY_DESCRIPTOR *)memMap, &mapKey, &despSize, &despVer);
  uefi_call_wrapper(SystemTable->BootServices->ExitBootServices, 2, ImageHandle, mapKey);

  bootInfo->RsdpPtr = rsdpPtr;
  bootInfo->MemMap = memMap;
  bootInfo->DespSize = despSize;
  bootInfo->MemMapSize = memMapSize / despSize;

  __asm__ volatile (
       "ld.d $sp, %0\n"
       "ld.d $a0, %0\n"
       "ld.d $t0, %1\n"
       "jr $t0\n"
       ::"m"(bootInfo), "m"(kernelMain)
       :"$t0");

  while (1);
  return EFI_SUCCESS;
}
