.PHONY: all

EFI_PLATFORM = 9
CFLAGS = -Ignu-efi/include -fshort-wchar
LDFLAGS = -shared -Bsymbolic -Lgnu-efi/lib/ -Tgnu-efi/lib/elf_loongarch64_efi.lds gnu-efi/lib/crt0-efi-loongarch64.o -lefi -lgnuefi
FORMAT = --target efi-app-loongarch64 --subsystem=10

all: loader.efi

%.efi: %.so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .rodata -j .rel \
		    -j .rela -j .rel.* -j .rela.* -j .rel* -j .rela* \
		    -j .areloc -j .reloc $(FORMAT) $*.so $@

%.so: %.o
	$(LD) $(LDFLAGS) $^ -o $@ $(LOADLIBES)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	if [ -f loader.efi ];then rm loader.efi; fi
	if [ -f loader.o ]; then rm loader.o; fi
