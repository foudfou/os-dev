# Automatically generate lists of sources using wildcards.
C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)
ARCH = 32

# TODO: Make sources dep on all header files.

# Convert the *.c filenames to *.o to give a list of object files to build
OBJ = ${C_SOURCES:.c=.o}

# Defaul build target
all: os-image

# Run bochs to simulate booting of our code.
run: all
	bochs

# FIXME gcc produces a 4MB kernel binary which bochs seems to refuse for 1_44 floppy.
# https://stackoverflow.com/questions/65037919/minimal-executable-size-now-10x-larger-after-linking-than-2-years-ago-for-tiny
run-kernel-bochs: all
	bochs -q 'boot:floppy' 'floppya: 1_44="os-image", status=inserted'

run-kernel-qemu: all
	qemu-system-x86_64 -fda os-image

# This is the actual disk image that the computer loads
# which is the combination of our compiled bootsector and kernel
os-image: boot/boot_sect.bin kernel.bin
	cat $^ > os-image

# This builds the binary of our kernel from two object files:
# 	- the kernel_entry,which jumps to main() in our kernel
# 	- the compiled C kernel
kernel.bin: kernel/kernel_entry.o ${OBJ}
# `-Ttext` locates text section at 0x1000, so our code knows to offset local
# address references from this origin, exactly liek `org 0x7c00`.
	ld -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
%.o : %.c ${HEADERS}
# `-fno-pie` to avoid: undefined reference to `_GLOBAL_OFFSET_TABLE_'.
# We also need the chain to be consistent: 32-bit
	gcc -fno-pie -m${ARCH} -ffreestanding -I. -c $< -o $@

# Assemble the kernel_entry.
%.o : %.asm
	nasm $< -f elf -o $@

%.bin : %.asm
	nasm $< -f bin -I 'boot/' -o $@

.PHONY: clean
clean:
		rm -fr *.bin *.dis *.o os-image
		rm -fr kernel/*.o boot/*.bin drivers/*.o


# Disassemble our kernel - might be useful for debugging.
kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

objdump-kernel:
	objdump -b binary --headers -f kernel.bin
