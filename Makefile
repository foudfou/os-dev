CC      = gcc
AS      = nasm
LD      = ld
CFLAGS  = -fno-pie -m32 -nostdlib -ffreestanding
LDFLAGS = -m elf_i386

# Automatically generate lists of sources using wildcards.
C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS   = $(wildcard kernel/*.h drivers/*.h)

# TODO: Make sources dep on all header files.

# Convert the *.c filenames to *.o to give a list of object files to build
OBJ = ${C_SOURCES:.c=.o}

# Defaul build target
all: os.img

# Run bochs to simulate booting of our code.
run: all
	bochs

# This is the actual disk image that the computer loads
# which is the combination of our compiled bootsector and kernel
os.img: boot/boot_sect.bin kernel.bin
	cat $^ | dd of=os.img bs=512 conv=sync

# This builds the binary of our kernel from two object files:
# 	- the kernel_entry,which jumps to main() in our kernel
# 	- the compiled C kernel
kernel.bin: kernel/kernel_entry.o kernel/isr.o ${OBJ}
# `-Ttext` locates text section at 0x1000, so our code knows to offset local
# address references from this origin, exactly liek `org 0x7c00`.
	$(LD) $(LDFLAGS) -o $@ -Ttext 0x1000 $^ --oformat binary

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
%.o : %.c ${HEADERS}
# `-fno-pie` to avoid: undefined reference to `_GLOBAL_OFFSET_TABLE_'.
# We also need the chain to be consistent: 32-bit
	$(CC) -I$(PWD) -c $< $(CFLAGS) -o $@

# Assemble the kernel_entry.
%.o : %.asm
	nasm $< -f elf -o $@

%.bin : %.asm
	nasm $< -f bin -I $(PWD)/boot -o $@

.PHONY: clean
clean:
	rm -fr *.bin *.dis *.o os.img
	rm -fr kernel/*.o boot/*.bin drivers/*.o


.PHONY: run-bochs
run-bochs: all
	bochs -q -f bochsrc

.PHONY: run-qemu
run-qemu: all
	qemu-system-x86_64 -fda os.img


# Disassemble our kernel - might be useful for debugging.
kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

objdump-kernel:
	objdump -b binary --headers -f kernel.bin
