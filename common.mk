ROOT_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

hex_boot_sect:
	od -t x1 -A n boot_sect.bin

run-bochs:
	bochs -q -f $(ROOT_DIR)/bochsrc

run-qemu:
	qemu-system-x86_64 -fda boot_sect.bin

clean:
	rm boot_sect.bin
