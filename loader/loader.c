#include <efi.h>
#include <efilib.h>

#define PAGE_SIZE     4096
#define KERNEL_ADDR   0xA00000
#define KERNEL_PATH   L"\\kernel.bin"

typedef struct
{
  EFI_SYSTEM_TABLE       *system;
  VOID                   *framebuffer;
  UINT32                 width;
  UINT32                 height;
} BootInfo;

BootInfo *boot_info = (BootInfo *)(KERNEL_ADDR - sizeof(BootInfo));

EFI_STATUS EFIAPI efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  InitializeLib(ImageHandle, SystemTable);

  EFI_STATUS                      status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *simple_filesystem;
  EFI_GRAPHICS_OUTPUT_PROTOCOL    *gop;
  EFI_FILE_PROTOCOL               *root, *kernel, *handle;
  EFI_LOADED_IMAGE_PROTOCOL       *loaded_image_protocol;
  UINTN                           kernel_size = 0xffffffffffffffff;
  VOID                            *kernel_main;
  EFI_PHYSICAL_ADDRESS            buffer = (EFI_PHYSICAL_ADDRESS)KERNEL_ADDR;
  EFI_TIME                        *time;

  UINTN                           memory_map_size = 0;
  EFI_MEMORY_DESCRIPTOR           *memory_map = NULL;
  UINTN                           map_key;
  UINTN                           descriptor_size;
  UINT32                          descriptor_version;

  EFI_GUID                 EfiAcpiTableGuid = ACPI_20_TABLE_GUID;
  EFI_CONFIGURATION_TABLE  *EfiConfigurationTable;
  UINT8                    *RsdpPtr = NULL;
  UINT8                    RsdpRevision;
  UINTN                    Index;

  gST = SystemTable;
  gBS = SystemTable->BootServices;
  gRT = SystemTable->RuntimeServices;

  for (Index = 0; Index < SystemTable->NumberOfTableEntries; Index++)
  {
    if (CompareGuid(&EfiAcpiTableGuid, &(SystemTable->ConfigurationTable[Index].VendorGuid)) == 0)
    {
      EfiConfigurationTable = &SystemTable->ConfigurationTable[Index];
      RsdpPtr = (UINT8 *)EfiConfigurationTable->VendorTable;

      Print(L"RSDP Pointer: %p\n", RsdpPtr);

      if(*(UINT8 *)(RsdpPtr) == 'R')
      {
        RsdpRevision = *(RsdpPtr + 15);
        Print(L"Signature found, RSDP Revision: %d\n", RsdpRevision);

        if (RsdpRevision < 2)  /* is revision 2 */
          Print(L"ERROR: RSDP version less than 2 is not supported.\n");
      }
      break;
    }
  }

  status = uefi_call_wrapper(gBS->SetWatchdogTimer, 4, 0, 0, 0, NULL);

  status = uefi_call_wrapper(gBS->LocateProtocol, 3, &gEfiGraphicsOutputProtocolGuid, NULL, (void **)&gop);

  status = uefi_call_wrapper(gBS->LocateProtocol, 3, &gEfiSimpleFileSystemProtocolGuid, NULL, (void **)&simple_filesystem);

  status = uefi_call_wrapper(simple_filesystem->OpenVolume, 2, simple_filesystem, &root);

  status = uefi_call_wrapper(gBS->HandleProtocol, 3, ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&loaded_image_protocol);
  status = uefi_call_wrapper(gBS->HandleProtocol, 3, loaded_image_protocol->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&simple_filesystem);

  status = uefi_call_wrapper(simple_filesystem->OpenVolume, 2, simple_filesystem, &kernel);

  status = uefi_call_wrapper(kernel->Open, 5, kernel, &handle, KERNEL_PATH, EFI_FILE_MODE_READ, 0);


  status = uefi_call_wrapper(handle->SetPosition, 2, handle, kernel_size);
  status = uefi_call_wrapper(handle->GetPosition, 2, handle, &kernel_size);
  status = uefi_call_wrapper(handle->SetPosition, 2, handle, 0);


  status = uefi_call_wrapper(gBS->AllocatePages, 4, AllocateAddress, EfiLoaderData, ((kernel_size + PAGE_SIZE) / PAGE_SIZE), &buffer);
  if(status == EFI_SUCCESS)
    Print(L"Allocated pages.\n");

  status = uefi_call_wrapper(handle->Read, 3, handle, &kernel_size, (VOID *)buffer);
  status = uefi_call_wrapper(handle->Close, 1, handle);

  status = uefi_call_wrapper(root->Close, 1, root);

  kernel_main = (VOID*) buffer;

  if(status == EFI_SUCCESS)
    Print(L"Parsed kernel size: %d start of binary in memory: %llx kernel_main at: %llx\n", kernel_size, buffer, kernel_main);


  status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiBootServicesData, sizeof(EFI_TIME), (VOID **)&time);
  status = uefi_call_wrapper(gRT->GetTime, 2, time, NULL);

  Print(L"Getting memory map and allocating pool...\n");

  status = uefi_call_wrapper(gBS->GetMemoryMap, 5, &memory_map_size, (EFI_MEMORY_DESCRIPTOR *)memory_map, &map_key, &descriptor_size, &descriptor_version);
   if(status == EFI_BUFFER_TOO_SMALL)
    Print(L"GetMemoryMap - EFI buffer too small.\n");
  memory_map_size += 1024 * descriptor_size;
  status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiLoaderData, memory_map_size, (void **)&memory_map);
  if(status == EFI_SUCCESS)
    Print(L"Allocated pool.\n");
  status = uefi_call_wrapper(gBS->GetMemoryMap, 5, &memory_map_size, (EFI_MEMORY_DESCRIPTOR *)memory_map, &map_key, &descriptor_size, &descriptor_version);
  if(status == EFI_SUCCESS)
    Print(L"GetMemoryMap.\n");
  status = uefi_call_wrapper(gBS->ExitBootServices, 2, ImageHandle, map_key);
  if(status == EFI_SUCCESS)
    Print(L"ExitBootServices.\n");

  boot_info->system = gST;
  boot_info->framebuffer = (VOID *)gop->Mode->FrameBufferBase;
  boot_info->width = gop->Mode->Info->HorizontalResolution;
  boot_info->height = gop->Mode->Info->VerticalResolution;

  Print(L"framebuffer address: %llx\n", boot_info->framebuffer);
  Print(L"Starting the kernel...\n");

  __asm__ volatile (
      "ld.d $sp, %0\n"
      "ld.d $t0, %1\n"
      "jr $t0\n"
      ::"m"(boot_info), "m"(kernel_main)
      :"$sp", "$t0");

  return EFI_SUCCESS;
}
