CFLAGS = -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar
LDFLAGS = -shared -Bsymbolic -L/usr/lib -T/usr/lib/elf_loongarch64_efi.lds /usr/lib/crt0-efi-loongarch64.o
LOADLIBES = -lefi -lgnuefi
FORMAT = --target efi-app-loongarch64 --subsystem=10

CC = $(PREFIX)gcc
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy

.PHONY: all
all: loader.efi

%.efi: %.so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .rodata -j .rel \
		    -j .rela -j .rel.* -j .rela.* -j .rel* -j .rela* \
		    -j .areloc -j .reloc $(FORMAT) $*.so $@

%.so: %.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LOADLIBES)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
