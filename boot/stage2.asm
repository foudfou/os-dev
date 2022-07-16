[org 0x7e00]
[bits 16]

KERNEL_OFFSET1 equ 0x1000       ; Memory offset to which we will temporarily
                                ; load our kernel.
KERNEL_OFFSET2 equ 0x100000     ; Final memory offset for the kernel.

global MEM_MAP:
MEM_MAP        equ 0xA000       ; Memory map. The number of entries will be
                                ; stored at the beginning.

start16:

    pop ax
    mov [BOOT_DRIVE], al

    mov bx, MSG_START_STAGE2
    call print_string

    mov dword [MEM_MAP], 0      ; Initialize memory map
    call e820_start             ; scan memory

    call load_kernel    ; Load our kernel

    mov bx, MSG_KERNEL_LOADED
    call print_string

    ; To load the kernel to 1MiB+ we currently use BIOS Int 15h, ah=87h "Copy
    ; Extended Memory". Other solutions include:
    ;
    ; 1. switch to unreal-mode (https://wiki.osdev.org/Unreal_Mode),
    ;
    ; 2. switch back and forth between real-mode to load from disk, and
    ; protected-mode to copy to higher memory
    ; (https://wiki.osdev.org/Real_Mode#x86_Assembly_Example) — that's what
    ; GRUB does,
    ;
    ; 3. if our kernel remains small enough that can it all be loaded from disk
    ; in real mode, we can simply copy it to himem from protected mode,
    ; e.g. `rep movsl`.
    ;
    ; 3. if our kernel is small and we want to copy it to 1MB, we can do it in
    ; real mode as we're able to address up to FFFF:FFFFh which is almost 64KB
    ; above 1MB (https://groups.google.com/g/alt.os.development/c/S2iZDBzHSW8).
    ;
    ; A floppy driver for protected mode would be too complicated.
    ;
    ; There's also a bit of a catch-22 as we can't know for sure if 1MB is
    ; usable until getting the memory map from e820: there can be memory hole
    ; or defect anywhere. The load/copy call would then fail.
    mov cx, (512 * KERNEL_SECTORS)
    call copy_extmem    ; Copy kernel to high memory

    mov bx, MSG_KERNEL_COPIED
    call print_string

    ; Poor attempt at enabling the A20 line. Properly detecting and enabling
    ; A20 is involved. So for now we'll deem sufficent to: 1. attempt to enable
    ; A20, 2. add a safeguard in the kernel when initializing physical memory
    ; management.
    ;
    ; Do this late as other interrupts might disable A20 again, like in VirtualBox.
    ;
    ; See https://wiki.osdev.org/A20
    ; https://www.win.tue.nl/~aeb/linux/kbd/A20.html
    mov ax, 0x2401              ; Activate A20-Gate
    int 15h

    mov bx, MSG_A20
    call print_string

    call switch_to_pm   ; Switch to protected mode, from which
                        ; we will not return

    jmp $                  ; Hang.

[bits 16]

load_kernel:
    mov bx, MSG_LOAD_KERNEL  ; Print a message to say we are loading the kernel
    call print_string

    mov bx, KERNEL_OFFSET1
    mov dh, KERNEL_SECTORS
    mov al, 1 + STAGE2_SECTORS + 1    ; Start reading from sector BL.
    mov dl, [BOOT_DRIVE]
    call disk_load

    ret

[bits 32]

; This is where we arrive after switching to and initialising protected mode.
start32:

    mov ebx, MSG_PROT_MODE      ; Use our 32-bit print routine to
    call print_string_pm        ; announce we are in protected mode

    mov eax, MEM_MAP            ; Pass physical memory map as argument to
    push eax                    ; kernel's main function
    jmp KERNEL_OFFSET2     ; Now jump to the address of our loaded
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
%include "copy_extmem.asm"
%include "print_string_pm.asm"
%include "switch_to_pm.asm"


; Global variables
BOOT_DRIVE         db 0
MSG_START_STAGE2   db "Starting stage2", 0xA, 0xD, 0
MSG_A20            db "A20 line enabled", 0xA, 0xD, 0
MSG_LOAD_KERNEL    db "Loading kernel into memory", 0xA, 0xD, 0
MSG_KERNEL_LOADED  db "Kernel loaded from disk", 0xA, 0xD, 0
MSG_KERNEL_COPIED  db "Kernel copied to dest memory", 0xA, 0xD, 0
MSG_SWITCH_PM      db "Switching to Protected Mode", 0xA, 0xD, 0
MSG_PROT_MODE      db "Successfully landed in 32-bit Protected Mode", 0


epilogue:
%if ($ - $$) < 512
%fatal "Bootloader stage2 too small"
%elif ($ - $$) > 1024
%fatal "Bootloader stage2 too large"
%endif

times (512 - ($ - $$) % 512) db 0
