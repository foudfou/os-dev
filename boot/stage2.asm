[org 0x7e00]
[bits 16]

start16:
    KERNEL_OFFSET equ 0x1000  ; This is the memory offset to which we will load our kernel

global MEM_MAP:
    MEM_MAP equ 0xA000        ; the number of entries will be stored at 0x8000

    pop ax
    mov [BOOT_DRIVE], al

    mov bx, MSG_START_STAGE2
    call print_string

    ; Poor attempt at enabling the A20 line. Properly detecting and enabling
    ; A20 is involved. So for now we'll deem sufficent to: 1. attempt to enable
    ; A20, 2. add a safeguard in the kernel when initializing physical memory
    ; management.
    ;
    ; See https://wiki.osdev.org/A20
    ; https://www.win.tue.nl/~aeb/linux/kbd/A20.html
    mov ax, 0x2401              ; Activate A20-Gate
    int 15h
    mov bx, MSG_A20
    call print_string

    mov dword [MEM_MAP], 0      ; Initialize memory map
    call e820_start             ; scan memory

    call load_kernel    ; Load our kernel

    call switch_to_pm   ; Switch to protected mode, from which
                        ; we will not return

    jmp $                  ; Hang.


[bits 16]

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

; Currently we can't load much more than 40 sectors without risking to
; override our boot_sect code in memory: 0x1000+(512*40)=0x6000,
; 0x1000+(512*60)=0x8800.
KERNEL_SECTORS equ 40  ; Number of sectors read from disk for the kernel

    mov bx, MSG_LOAD_KERNEL  ; Print a message to say we are loading the kernel
    call print_string

    mov bx, KERNEL_OFFSET
    mov dh, KERNEL_SECTORS
    mov al, 1 + STAGE2_SECTORS + 1    ; Start reading from sector BL.
    mov dl, [BOOT_DRIVE]
    call disk_load          ; (i.e. our kernel code) to address KERNEL_OFFSET

    mov bx, MSG_LOADED_KERNEL
    call print_string

    ret

[bits 32]

; This is where we arrive after switching to and initialising protected mode.
start32:

    mov ebx, MSG_PROT_MODE      ; Use our 32-bit print routine to
    call print_string_pm        ; announce we are in protected mode

    mov eax, MEM_MAP            ; Pass physical memory map as argument to
    push eax                    ; kernel's main function
    jmp KERNEL_OFFSET      ; Now jump to the address of our loaded
                           ; kernel code, assume the brace position,
                           ; and cross your fingers. Here we go!

    jmp $                  ; Hang.


[bits 16]

; Include our useful, hard-earned routines
%include "defs.asm"
%include "print_string.asm"
%include "disk_load.asm"
%include "e820.asm"
%include "gdt.asm"
%include "print_string_pm.asm"
%include "switch_to_pm.asm"


; Global variables
BOOT_DRIVE         db 0
MSG_START_STAGE2   db "Starting stage2", 0xA, 0xD, 0
MSG_A20            db "A20 line enabled", 0xA, 0xD, 0
MSG_LOAD_KERNEL    db "Loading kernel into memory", 0xA, 0xD, 0
MSG_LOADED_KERNEL  db "Kernel loaded", 0xA, 0xD, 0
MSG_SWITCH_PM      db "Switching to Protected Mode", 0xA, 0xD, 0
MSG_PROT_MODE      db "Successfully landed in 32-bit Protected Mode", 0


epilogue:
%if ($ - $$) < 512
%fatal "Bootloader stage2 too small"
%elif ($ - $$) > 1024
%fatal "Bootloader stage2 too large"
%endif

times (512 - ($ - $$) % 512) db 0
