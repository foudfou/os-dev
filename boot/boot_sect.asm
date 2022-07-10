; A boot sector that boots a C kernel in 32-bit protected mode
[org 0x7c00]
    KERNEL_OFFSET equ 0x1000  ; This is the memory offset to which we will load our kernel

global PMEM_ENT:
    PMEM_ENT equ 0x8000        ; the number of entries will be stored at 0x8000

    mov [BOOT_DRIVE], dl      ; BIOS stores our boot drive in DL, so it's
                              ; best to remember this for later.

    mov bp, 0x9000              ; Set-up the stack.
    mov sp, bp
    xor bp, bp                  ; Set EBP to NULL for stack tracing's use.

    mov bx, MSG_REAL_MODE       ; Announce that we are starting
    call print_string           ; booting from 16-bit real mode

    ; Now would be a good time to detect and enable the A20 line. But,
    ; 1. properly detecting and enabling A20 is involved, 2. our boot sector is
    ; already nearing 512B. We might consider making a 2-stage bootloader
    ; later, but for now we'll deem sufficent to: 1. attempt to enable A20,
    ; 2. add a safeguard in the kernel when initializing physical memory
    ; management.
    ;
    ; See https://wiki.osdev.org/A20
    ; https://www.win.tue.nl/~aeb/linux/kbd/A20.html
    ;
    ; Bootloader stage2: https://github.com/gynvael/osdev/tree/master/src/boot
    ; https://github.com/revenn/OSdev/blob/master/stage2.asm
    mov ax, 0x2401              ; Activate A20-Gate
    int 15h

    mov dword [PMEM_ENT], 0     ; Initialize memory map
    call pmem_e820              ; scan memory

    call load_kernel    ; Load our kernel

    call switch_to_pm   ; Switch to protected mode, from which
                        ; we will not return

    jmp $

; Include our useful, hard-earned routines
%include "print_string.asm"
%include "disk_load.asm"
%include "gdt.asm"
%include "print_string_pm.asm"
%include "switch_to_pm.asm"
%include "pmem_e820.asm"

[bits 16]

; Currently we can't load much more than 40 sectors without risking to override
; our boot_sect code in memory: 0x1000+(512*40)=0x6000, 0x1000+(512*60)=0x8800.
KERNEL_SECTORS equ 40  ; Number of sectors read from disk for the kernel

; load_kernel. To keep things simple we're loading to lower memory.
;
; To load the kernel to 1MiB+ we have to either: 1. use BIOS Int 15h, ah=87h
; "Copy Extended Memory" (https://unix.stackexchange.com/a/222770/145419),
; 2. switch to unreal-mode (https://wiki.osdev.org/Unreal_Mode), 3. switch back
; and forth between real-mode to load from disk, and protected-mode to copy to
; higher memory (https://wiki.osdev.org/Real_Mode#x86_Assembly_Example) —
; that's what GRUB does. We could get away with real-mode for a small kernel as
; we're able to address up to FFFF:FFFFh which is almost 64KB above 1MB
; (https://groups.google.com/g/alt.os.development/c/S2iZDBzHSW8). A floppy
; driver for protected mode would be too complicated. It's also worth noting
; that we can't know for sure if 1MB is usable until getting the memory map
; from e820: there can be memory hole or defect anywhere. The load/copy call
; would then fail.
load_kernel:
    mov bx, MSG_LOAD_KERNEL  ; Print a message to say we are loading the kernel
    call print_string

    mov bx, KERNEL_OFFSET   ; Set-up parameters for our disk_load routine, so
    mov dh, KERNEL_SECTORS  ; that we load the first KERNEL_SECTORS sectors
    mov dl, [BOOT_DRIVE]    ; (excluding the boot sector) from the boot disk
    call disk_load          ; (i.e. our kernel code) to address KERNEL_OFFSET

    ret

[bits 32]

; This is where we arrive after switching to and initialising protected mode.
BEGIN_PM:

    mov ebx, MSG_PROT_MODE      ; Use our 32-bit print routine to
    call print_string_pm        ; announce we are in protected mode

    mov eax, PMEM_ENT           ; Pass physical memory map as argument to
    push eax                    ; kernel's main function
    jmp KERNEL_OFFSET      ; Now jump to the address of our loaded
                           ; kernel code, assume the brace position,
                           ; and cross your fingers. Here we go!

    jmp $                  ; Hang.

; Global variables
BOOT_DRIVE       db 0
MSG_REAL_MODE    db "Started in 16-bit Real Mode", 0xA, 0xD, 0
MSG_LOAD_KERNEL  db "Loading kernel into memory", 0xA, 0xD, 0
MSG_PROT_MODE    db "Successfully landed in 32-bit Protected Mode", 0

; Bootsector padding
times 510-($-$$) db 0
dw 0xaa55
