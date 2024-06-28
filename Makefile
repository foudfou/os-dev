# Using https://aur.archlinux.org/packages/i386-elf-gcc cross-compiler.
TOOLPREFIX = i386-elf-

CC      = $(TOOLPREFIX)gcc
AS      = nasm
LD      = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

# From https://github.com/dreamos82/Osdev-Notes/blob/master/01_Build_Process/02_Overview.md
# `-fno-pie` to avoid: undefined reference to `_GLOBAL_OFFSET_TABLE_'.
# We also need the chain to be consistent: 32-bit
CFLAGS  = -std=c17 -fno-pie -nostdlib -ffreestanding
# `-mno-red-zone`: disables the red-zone, a 128 byte region reserved on the
# stack for optimizations. Hardware interrupts are not aware of the red-zone,
# and will clobber it. So we need to disable it in the kernel or we'll lose
# data.
#
# `-fno-omit-frame-pointer`: Sometimes the compiler will skip creating a new
# stack frame for optimization reasons. This will mess with stack traces, and
# only increases the memory usage by a few bytes here and there.
CFLAGS += -mno-red-zone -fno-omit-frame-pointer
CFLAGS += -ggdb
LDFLAGS = -static
ASFLAGS = -g
LDS     = kernel.lds

K=kernel
U=user

# Automatically generate lists of sources using wildcards.
C_SRCS  = $(wildcard $K/lib/*.c $K/*.c $K/drivers/*.c)
HEADERS = $(wildcard $K/lib/*.h $K/*.h $K/drivers/*.h)
OBJS    = $(C_SRCS:.c=.o)

# TODO: Make sources depend on all header files.

HEADER_DEFS = \
	$K/idt_defs.h\
	$K/paging_defs.h\
	$K/syscall_defs.h

# For user programs to link against
ULIB = $U/syscall.o

# Defaul build target
all: os.img

# Run bochs to simulate booting of our code.
run: all
	bochs

# Sectors loaded from floppy. As per asm copy_extmem (INT 15h,87h XT-286, AT),
# we shouldn't be able to load more than 40 sectors. Qemu doesn't seem to
# bother but bochs limits to 72 (9000h). Ideally we would allow ourselves the
# max of 0xFFFF bytes ~ 127 * 512.
KERNEL_SECTORS := 72

# This is the actual disk image that the computer loads
# which is the combination of our compiled bootsector and kernel
os.img: boot/stage1.bin boot/stage2.bin kernel.bin $(ULIB)
# Ensure our kernel file doesn't exceed what the bootloader will load from the disk.
	bash -c '[ $$(stat -c %s kernel.bin) -lt $$(expr 512 \* $(KERNEL_SECTORS)) ]'
	cat $^ /dev/zero | dd of=$@ bs=1k count=1440 iflag=fullblock

fs.img: os.img
	dd if=/dev/zero of=fs.img count=10000
	dd if=os.img of=fs.img conv=notrunc

# Linking ORDER MATTERS!
OBJS := $K/kernel_entry.o $(OBJS) $K/isr.o $K/gdt_load.o $K/swtch.o

# This builds the binary of our kernel from two object files:
# 	- the kernel_entry,which jumps to main() in our kernel
# 	- the compiled C kernel
kernel.bin: $(HEADER_DEFS) $(OBJS) $U/initcode $U/init
# `-b <input-format>` specifies a new binary format for object files
# after this option.
	$(LD) $(LDFLAGS) -o kernel.elf -T $(LDS) \
		--oformat=elf32-i386 $(OBJS) \
		-b binary $U/init \
		--print-map > kernel.map
# Note binary (ld or objcopy discards all symbols and relocation information).
	$(OBJCOPY) -S -O binary kernel.elf $@

# initcode is superseded by init for now.
$U/initcode: $U/initcode.asm $(ULIB)
	$(AS) $(ASFLAGS) $U/initcode.asm -f elf32 -o $U/initcode.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $U/initcode.out $U/initcode.o $(ULIB)
	$(OBJCOPY) -S -O binary $U/initcode.out $U/initcode

$U/init: $U/init.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $U/init.out $^
	$(OBJCOPY) -S -O binary $U/init.out $U/init

# C init doesn't need gcc stack alignment and stack protection.
# https://reverseengineering.stackexchange.com/questions/15173/what-is-the-purpose-of-these-instructions-before-the-main-preamble
# https://stackoverflow.com/questions/38781118/why-is-gcc-pushing-an-extra-return-address-on-the-stack
# https://stackoverflow.com/questions/45423338/whats-up-with-gcc-weird-stack-manipulation-when-it-wants-extra-stack-alignment
UCFLAGS  = -fno-stack-protector
UCFLAGS += -maccumulate-outgoing-args # -mpreferred-stack-boundary=2

$U/init.o: $U/init.c $(HEADERS)
	$(CC) -I. -c $< $(CFLAGS) $(UCFLAGS) -o $@

# `-fstack-protector`: requires that we implement __stack_chk_*
KCFLAGS = -fstack-protector

%.o: %.c $(HEADERS)
	$(CC) -I. -Ikernel -c $< $(CFLAGS) $(KCFLAGS) -o $@

%.o : %.asm
	$(AS) $(ASFLAGS) $< -f elf32 -o $@

%.bin : %.asm
	$(AS) $< -f bin -I ./boot -o $@ \
		-dKERNEL_SECTORS=$(KERNEL_SECTORS)

%_defs.h: %_defs.asm
	sed -e 's/%define/#define/' -e 's/;\+/\/\//' $< > $@

.PHONY: clean
clean:
	rm -fr *.bin *.elf *.dis *.o os.img *.map
	rm -fr boot/*.bin $K/*.o drivers/*.o $K/*.out
	rm -fr $K/*_defs.h
	rm -fr $U/initcode $U/init $U/*.o $U/*.out
	rm -fr $(OBJS)


.PHONY: run-bochs
run-bochs: all fs.img
	bochs -q -f bochsrc

QEMUOPTS = -fda os.img
# As of qemu 6.2, `-smp cpus=2` is `-smp sockets=1,cores=2`, i.e. 1 cpu.
# QEMUOPTS += -smp sockets=2,cores=2
QEMUOPTS += -drive file=fs.img,index=1,media=disk,format=raw

# Exit curses with Alt + 2
.PHONY: run-qemu
run-qemu: all fs.img
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
