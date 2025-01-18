.PHONY: loader kernel

all: loader kernel

loader:
	make -C loader

kernel:
	make -C kernel

img: loader kernel
	dd if=/dev/zero of=akarios.img bs=16k count=1024
	mformat -i akarios.img ::
	mcopy -i akarios.img loader/loader.efi ::/
	mcopy -i akarios.img kernel/kernel.bin ::/

qemu: img
	qemu-system-loongarch64 \
	-bios /usr/share/qemu/edk2-loongarch64-code.fd \
	-serial stdio \
	-drive format=raw,file=akarios.img \
	-device virtio-gpu-pci \
	-device nec-usb-xhci,id=xhci,addr=0x1b \
	-device usb-tablet,id=tablet,bus=xhci.0,port=1 \
	-device usb-kbd,id=keyboard,bus=xhci.0,port=2

qemu-debug: img
	qemu-system-loongarch64 \
	-bios /usr/share/qemu/edk2-loongarch64-code.fd \
	-serial stdio \
	-drive format=raw,file=akarios.img \
	-device virtio-gpu-pci \
	-device nec-usb-xhci,id=xhci,addr=0x1b \
	-device usb-tablet,id=tablet,bus=xhci.0,port=1 \
	-device usb-kbd,id=keyboard,bus=xhci.0,port=2 \
	-S -s
