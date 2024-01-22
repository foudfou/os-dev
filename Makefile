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
  -fstack-protector \
  -ggdb
LDFLAGS = -static
ASFLAGS = -g
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
KERNEL_SECTORS := 50

# This is the actual disk image that the computer loads
# which is the combination of our compiled bootsector and kernel
os.img: boot/stage1.bin boot/stage2.bin kernel.bin
# Ensure our kernel file doesn't exceed what the bootloader will load from the disk.
	bash -c '[ $$(stat -c %s kernel.bin) -lt $$(expr 512 \* $(KERNEL_SECTORS)) ]'
	cat $^ /dev/zero | dd of=$@ bs=1k count=1440 iflag=fullblock

user/initcode: user/initcode.asm
	$(AS) $(ASFLAGS) user/initcode.asm -f elf32 -o user/initcode.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o user/initcode.out user/initcode.o
	$(OBJCOPY) -S -O binary user/initcode.out user/initcode

# Linking ORDER MATTERS!
OBJS := kernel/kernel_entry.o ${OBJS} kernel/isr.o kernel/gdt_load.o kernel/swtch.o

# This builds the binary of our kernel from two object files:
# 	- the kernel_entry,which jumps to main() in our kernel
# 	- the compiled C kernel
kernel.bin: ${OBJS} user/initcode
# `-b <input-format>` specifies a new binary format for object files
# after this option.
	$(LD) $(LDFLAGS) -o kernel.elf -T $(LDS) \
		--oformat=elf32-i386 $(OBJS) \
		-b binary user/initcode \
		--print-map > kernel.map
# Note binary (ld or objcopy discards all symbols and relocation information).
	$(OBJCOPY) -S -O binary kernel.elf $@

# Generic rule for compiling C code to an object file
# For simplicity , we C files depend on all header files .
%.o : %.c ${HEADERS}
# `-fno-pie` to avoid: undefined reference to `_GLOBAL_OFFSET_TABLE_'.
# We also need the chain to be consistent: 32-bit
	$(CC) -I$(PWD) -c $< $(CFLAGS) -o $@

%.o : %.asm
	$(AS) $(ASFLAGS) $< -f elf32 -o $@

%.bin : %.asm
	$(AS) $< -f bin -I ./boot -o $@

.PHONY: clean
clean:
	rm -fr *.bin *.elf *.dis *.o os.img *.map
	rm -fr boot/*.bin kernel/*.o drivers/*.o kernel/*.out user/initcode
	rm -fr $(OBJS)


.PHONY: run-bochs
run-bochs: all
	bochs -q -f bochsrc

QEMUOPTS = -fda os.img
# As of qemu 6.2, `-smp cpus=2` is `-smp sockets=1,cores=2`, i.e. 1 cpu.
# QEMUOPTS += -smp sockets=2,cores=2

# Exit curses with Alt + 2
.PHONY: run-qemu
run-qemu: all
# -nographic disables graphic output
	qemu-system-i386 $(QEMUOPTS) -display curses # -m 4G

# Exit with Ctl-A X
.PHONY: run-qemu-nogr
run-qemu-nogr: all
	qemu-system-i386 $(QEMUOPTS) -nographic # -serial pty -monitor pty

.PHONY: run-qemu-gdb
run-qemu-gdb: all
	@printf "==================================\n"
	@printf "make run-gdb-attach\n"
	@printf "==================================\n"
	qemu-system-i386 $(QEMUOPTS) -S -s

.PHONY: run-gdb-attach
run-gdb-attach:
	gdb -x .gdbinit

.PHONY: run-qemu-log
run-qemu-log: all
	qemu-system-i386 $(QEMUOPTS) -serial stdio -no-reboot -d int,cpu_reset

# Disassemble our kernel
kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

objdump-kernel-bin:
	objdump -D -b binary -m i386 kernel.bin

objdump-kernel-elf:
	objdump -D kernel.elf
