# Using https://aur.archlinux.org/packages/i386-elf-gcc cross-compiler.
TOOLPREFIX = i386-elf-

CC      = $(TOOLPREFIX)gcc
AS      = nasm
LD      = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

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
CFLAGS  = -std=c17 -fno-pie -nostdlib -ffreestanding \
  -mno-red-zone -fno-omit-frame-pointer \
  -fstack-protector
LDFLAGS = -static
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

# Sectors loaded from floppy
KERNEL_SECTORS := 40

# This is the actual disk image that the computer loads
# which is the combination of our compiled bootsector and kernel
os.img: boot/stage1.bin boot/stage2.bin kernel.bin
# Ensure our kernel file doesn't exceed what the bootloader will load from the disk.
	bash -c '[ $$(stat -c %s kernel.bin) -lt $$(expr 512 \* $(KERNEL_SECTORS)) ]'
	cat $^ /dev/zero | dd of=$@ bs=1k count=1440 iflag=fullblock

# This builds the binary of our kernel from two object files:
# 	- the kernel_entry,which jumps to main() in our kernel
# 	- the compiled C kernel
kernel.bin: kernel/kernel_entry.o kernel/isr.o ${OBJS}
# `-Ttext` locates text section at 0x1000, so our code knows to offset local
# address references from this origin, exactly liek `org 0x7c00`.
	$(LD) $(LDFLAGS) -o $@ -T$(LDS) $^ --oformat binary -M > kernel.map

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
%.o : %.c ${HEADERS}
# `-fno-pie` to avoid: undefined reference to `_GLOBAL_OFFSET_TABLE_'.
# We also need the chain to be consistent: 32-bit
	$(CC) -I$(PWD) -c $< $(CFLAGS) -o $@

# Assemble the kernel_entry.
%.o : %.asm
	$(AS) $< -f elf32 -o $@

%.bin : %.asm
	$(AS) $< -f bin -I ./boot -o $@

.PHONY: clean
clean:
	rm -fr *.bin *.dis *.o os.img *.map
	rm -fr $(OBJS)


.PHONY: run-bochs
run-bochs: all
	bochs -q -f bochsrc

# Exit curses with Alt + 2
.PHONY: run-qemu
run-qemu: all
	qemu-system-x86_64 -fda os.img -display curses


# Disassemble our kernel - might be useful for debugging.
kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

objdump-kernel:
	objdump -b binary --headers -f kernel.bin

# https://stackoverflow.com/a/34424194
objdump-boot-16:
	objdump -D -Mintel,i8086 -b binary -m i386  boot/boot_sect.bin

objdump-boot-32:
	objdump -D -Mintel,i386 -b binary -m i386 boot/boot_sect.bin
