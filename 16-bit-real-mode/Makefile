chap2:
	bochs

chap3:
# `-f bin` to produce raw machine code
	nasm boot_sect-chap3.asm -f bin -o boot_sect.bin

hex_boot_sect:
	od -t x1 -A n boot_sect.bin

run-bochs:
	bochs -q

run-qemu:
	qemu-system-x86_64 -fda boot_sect.bin

chap3-1:
	nasm boot_sect-chap3-1.asm -f bin -o boot_sect.bin

chap3-2:
	nasm boot_sect-chap3-2.asm -f bin -o boot_sect.bin

chap3-3:
	nasm boot_sect-chap3-3.asm -f bin -o boot_sect.bin

chap3-4-5:
	nasm boot_sect-chap3-4-5.asm -f bin -o boot_sect.bin

chap3-4-6:
	nasm -w+x boot_sect-chap3-4-6.asm -f bin -o boot_sect.bin

chap3-5-1:
	nasm boot_sect-chap3-5-1.asm -f bin -o boot_sect.bin

chap3-6-1:
	nasm boot_sect-chap3-6-1.asm -f bin -o boot_sect.bin

chap3-6-4:
	nasm boot_sect-chap3-6-4.asm -f bin -o boot_sect.bin

clean:
	rm boot_sect.bin
