CC = clang
LD = lld-link
EFI_PLATFORM = 9

CFLAGS = -I../3rdparty/yoppeh-efi -target loongarch64-unknown-windows
LDFLAGS = -entry:efiMain -subsystem:efi_application

loader.efi: loader.o
	$(LD) $(LDFLAGS) $^ -out:$@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm loader.efi loader.o
