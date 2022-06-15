CC      = gcc
AS      = nasm
LD      = ld

# From https://github.com/dreamos82/Osdev-Notes/blob/master/01_Build_Process/02_Overview.md
#
# `-mno-red-zone`: disables the red-zone, a 128 byte region reserved on the
# stack for optimizations. Hardware interrupts are not aware of the red-zone,
# and will clobber it. So we need to disable it in the kernel or we'll lose
# data.
#
# `-fno-stack-protector`: Disables stack protector checks, which use the
# compiler library to check for stack smashing attacks. Since we're not
# including the standard libaries, we can't use this unless we implement the
# functions ourselves. [But we implement it ourselves]
CFLAGS  = -std=c17 -fno-pie -m32 -nostdlib -ffreestanding \
  -mno-red-zone -fno-builtin -fno-omit-frame-pointer
LDFLAGS = -m elf_i386 -static
LDS     = kernel.lds

# CC      = clang
# AS      = nasm
# LD      = clang
# CFLAGS  = -std=c17 -fno-pie -nostdlib -ffreestanding -mno-red-zone -fno-stack-protector -W -Wall
# LDFLAGS = -nostdlib -mcmodel=kernel -nopie
# LDS     = kernel.lds


# Automatically generate lists of sources using wildcards.
C_SRCS  = $(wildcard lib/*.c kernel/*.c drivers/*.c)
HEADERS = $(wildcard lib/*.h kernel/*.h drivers/*.h)
OBJS    = ${C_SRCS:.c=.o}

# TODO: Make sources dep on all header files.

# Defaul build target
all: os.img

# Run bochs to simulate booting of our code.
run: all
	bochs

# This is the actual disk image that the computer loads
# which is the combination of our compiled bootsector and kernel
os.img: boot/boot_sect.bin kernel.bin
	cat $^ /dev/zero | dd of=$@ bs=1k count=1440

# This builds the binary of our kernel from two object files:
# 	- the kernel_entry,which jumps to main() in our kernel
# 	- the compiled C kernel
kernel.bin: kernel/kernel_entry.o kernel/isr.o ${OBJS}
# `-Ttext` locates text section at 0x1000, so our code knows to offset local
# address references from this origin, exactly liek `org 0x7c00`.
	$(LD) $(LDFLAGS) -o $@ -T$(LDS) $^ --oformat binary

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
%.o : %.c ${HEADERS}
# `-fno-pie` to avoid: undefined reference to `_GLOBAL_OFFSET_TABLE_'.
# We also need the chain to be consistent: 32-bit
	$(CC) -I$(PWD) -c $< $(CFLAGS) -o $@

# Assemble the kernel_entry.
%.o : %.asm
	$(AS) $< -f elf -o $@

%.bin : %.asm
	$(AS) $< -f bin -I ./boot -o $@

.PHONY: clean
clean:
	rm -fr *.bin *.dis *.o os.img
	rm -fr $(OBJS)


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
