## Kernel

all: os-image

# # FIXME gcc produces a 4MB kernel binary which bochs seems to refuse for 1_44 floppy.
# # https://stackoverflow.com/questions/65037919/minimal-executable-size-now-10x-larger-after-linking-than-2-years-ago-for-tiny
# run-kernel-bochs:
# 	bochs -q 'boot:floppy' 'floppya: 1_44="os-image", status=inserted'

run-kernel-qemu:
	qemu-system-x86_64 -fda os-image

# This is the actual disk image that the computer loads,
# which is the combination of our compiled bootsector and kernel
os-image: boot_sect-kernel.bin kernel.bin
	cat $^ > $@

# This builds the binary of our kernel from two object files :
# - the kernel_entry , which jumps to main () in our kernel
# - the compiled C kernel
kernel.bin: kernel_entry.o kernel.o
# `-Ttext` locates text section at 0x1000, so our code knows to offset local
# address references from this origin, exactly liek `org 0x7c00`.
	ld -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

# Build our kernel object file.
# For simplicity , we C files depend on all header files .
kernel.o: kernel.c
	gcc -ffreestanding -c $< -o $@
# `-fno-pie` to avoid: undefined reference to `_GLOBAL_OFFSET_TABLE_'.
# We also need the chain to be consistent: 32-bit
	gcc -fno-pie -m32 -ffreestanding -c $< -o $@

# Build our kernel entry object file.
kernel_entry.o: kernel_entry.asm
	nasm $< -f elf -o $@

# Assemble the boot sector to raw machine code
# The -I options tells nasm where to find our useful assembly
boot_sect-kernel.bin: boot_sect-kernel.asm
	nasm $< -f bin -I '../16-bit-real-mode/' -o $@

clean:
	rm -fr *.bin *.dis *.o os-image
	rm -fr kernel/*.o boot/*.bin drivers/*.o

# Disassemble our kernel - might be useful for debugging.
kernel.dis : kernel.bin
	ndisasm -b 32 $< > $@

objdump-kernel:
	objdump -b binary --headers -f kernel.bin


## Previous chapters

chap4-5:
	nasm boot_sect-switch-chap4-5.asm -f bin -o boot_sect.bin

## Basic compilation

# gcc -ffreestanding -c basic.c -o basic.o
# ld -o basic.bin -Ttext 0x0 --oformat binary basic.o
# objdump -d basic.o
# ndisasm -b 32 basic.bin | head -10
# rm basic.bin basic.o
