; Inspired by https://github.com/revenn/OSdev/blob/master/stage1.asm
; https://github.com/gynvael/osdev/blob/master/src/boot/stage1.asm
[bits 16]
[org 0x7c00]
    ; Setup the segments and stack. A typical location for a stack is
    ; 0000:7C00, right before the bootloader.
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x07c00              ; Set-up stack.
    sti

    mov [BOOT_DRIVE], dl        ; BIOS stores our boot drive in DL

STAGE2_OFFSET equ 0x7e00 ; Memory offset to which we will load stage2

    mov bx, MSG_LOAD_STAGE2
    call print_string

    call load_stage2

    push word [BOOT_DRIVE]

    jmp STAGE2_OFFSET

    jmp $

load_stage2:
    ; To write to a higher value than 0xffff, we need to use ES as the pointer
    ; to buffer is defined by ES:BX. But one consequence is that we would later
    ; have to do a far JUMP, i.e. `jmp word 0x2000:0x0000` which would also set
    ; CS to 0x2000 and complicates things in stages2.
    mov bx, STAGE2_OFFSET

    ; Number of sectors. With the nop opcodes, this forms the marker
    ; "\xb6\xXX\x90\x90" which could be used by some external script to replace
    ; the exact size of stage2
    mov dh, STAGE2_SECTORS
    nop
    nop
    mov al, 0x02     ; Start reading from sector BL.
    mov dl, [BOOT_DRIVE]
    call disk_load

    ret


%include "defs.asm"
%include "print_string.asm"
%include "disk_load.asm"

; Global variables
BOOT_DRIVE       db 0
MSG_LOAD_STAGE2  db "Loading stage2", 0xA, 0xD, 0


epilogue:
%if ($ - $$) > 510
%fatal "Bootloader stage1 exceed 512 bytes."
%endif

times 510 - ($ - $$) db 0
db 0x55
db 0xAA
