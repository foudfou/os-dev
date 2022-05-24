include ../common.mk

chap4-5:
	nasm boot_sect-switch-chap4-5.asm -f bin -o boot_sect.bin

# gcc -ffreestanding -c basic.c -o basic.o
# ld -o basic.bin -Ttext 0x0 --oformat binary basic.o
# objdump -d basic.o
# ndisasm -b 32 basic.bin | head -10
# rm basic.bin basic.o

kernel_entry.o:
	nasm kernel_entry.asm -f elf -o kernel_entry.o

kernel.bin: kernel_entry.o
# `-fno-pie` to avoid: undefined reference to `_GLOBAL_OFFSET_TABLE_'.
# We also need the chain to be consistent: 32-bit
	gcc -fno-pie -m32 -ffreestanding -c kernel.c -o kernel.o
# `-Ttext` locates text section at 0x1000, so our code knows to offset local
# address references from this origin, exactly liek `org 0x7c00`.
	ld -m elf_i386 -o kernel.bin -Ttext 0x1000 kernel_entry.o kernel.o --oformat binary

objdump-kernel:
	objdump -b binary --headers -f kernel.bin

boot_sect-kernel.bin:
	nasm boot_sect-kernel.asm -f bin -o boot_sect-kernel.bin

os-image: boot_sect-kernel.bin kernel.bin
	cat boot_sect-kernel.bin kernel.bin > os-image

# # FIXME gcc produces a 4MB kernel binary which bochs seems to refuse for 1_44 floppy.
# # https://stackoverflow.com/questions/65037919/minimal-executable-size-now-10x-larger-after-linking-than-2-years-ago-for-tiny
# run-kernel-bochs:
# 	bochs -q 'boot:floppy' 'floppya: 1_44="os-image", status=inserted'

run-kernel-qemu:
	qemu-system-x86_64 -fda os-image

clean-kernel:
	rm kernel_entry.o kernel.o kernel.bin boot_sect-kernel.bin os-image
