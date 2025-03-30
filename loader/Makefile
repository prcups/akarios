CFLAGS = -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -I../3rdparty/gnu-efi/inc
LDFLAGS = -shared -Bsymbolic -L../3rdparty/gnu-efi/loongarch64/gnuefi/ -L../3rdparty/gnu-efi/loongarch64/lib/ -T../3rdparty/gnu-efi/gnuefi/elf_loongarch64_efi.lds ../3rdparty/gnu-efi/loongarch64/gnuefi/crt0-efi-loongarch64.o
LOADLIBES = -lefi -lgnuefi
FORMAT = --target efi-app-loongarch64 --subsystem=10

CC = $(PREFIX)gcc
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy

.PHONY: all
all: ../3rdparty/gnu-efi/loongarch64 loader.efi

../3rdparty/gnu-efi/loongarch64:
	CROSS_COMPILE=$(PREFIX) ARCH=loongarch64 make -C ../3rdparty/gnu-efi

%.efi: %.so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .rodata -j .rel \
		    -j .rela -j .rel.* -j .rela.* -j .rel* -j .rela* \
		    -j .areloc -j .reloc $(FORMAT) $*.so $@

%.so: %.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LOADLIBES)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
