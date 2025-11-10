.PHONY: loader kernel

export CC = $(PREFIX)gcc
export CXX = $(PREFIX)g++
export LD = $(PREFIX)ld
export OBJCOPY = $(PREFIX)objcopy

all: loader kernel

loader:
	make -C loader

kernel:
	make -C kernel

img: loader kernel
	dd if=/dev/zero of=akarios.img bs=512 count=20000
	parted akarios.img -s -a min mklabel gpt
	parted akarios.img -s -a min mkpart EFI FAT16 2048s 16350s
	parted akarios.img -s -a min toggle 1 boot

	dd if=/dev/zero of=/tmp/akarios-c.img bs=512 count=16300
	mformat -i /tmp/akarios-c.img ::
	mcopy -i /tmp/akarios-c.img loader/loader.efi ::/
	mcopy -i /tmp/akarios-c.img kernel/kernel.bin ::/
	mcopy -i /tmp/akarios-c.img startup.nsh ::/
	dd if=/tmp/akarios-c.img of=akarios.img bs=512 count=16300 seek=2048 conv=notrunc

qemu: img
	qemu-system-loongarch64 \
	-bios /usr/share/qemu/edk2-loongarch64-code.fd \
	-drive id=disk,file=akarios.img,if=none \
	-device ahci,id=ahci \
	-device ide-hd,drive=disk,bus=ahci.0 \
	-device virtio-gpu-pci \
	-device nec-usb-xhci,id=xhci,addr=0x1b \
	-device usb-tablet,id=tablet,bus=xhci.0,port=1 \
	-device usb-kbd,id=keyboard,bus=xhci.0,port=2

qemu-debug: img
	qemu-system-loongarch64 \
	-bios /usr/share/qemu/edk2-loongarch64-code.fd \
	-drive id=disk,format=raw,file=akarios.img,if=none \
	-device nvme,serial=deadbeef,drive=disk \
	-device virtio-gpu-pci \
	-device nec-usb-xhci,id=xhci,addr=0x1b \
	-device usb-tablet,id=tablet,bus=xhci.0,port=1 \
	-device usb-kbd,id=keyboard,bus=xhci.0,port=2 \
	-S -s

clean:
	cd kernel && make clean
	cd loader && make clean
