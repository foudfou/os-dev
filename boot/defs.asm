STAGE2_SECTORS equ 2  ; Number of sectors read from disk for stage2

; Currently we can't load much more than 40 sectors without risking to
; override our boot_sect code in memory: 0x1000+(512*40)=0x6000,
; 0x1000+(512*60)=0x8800.
KERNEL_SECTORS equ 40  ; Number of sectors read from disk for the kernel
