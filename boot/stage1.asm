; Inspired by https://github.com/revenn/OSdev/blob/master/stage1.asm
; https://github.com/gynvael/osdev/blob/master/src/boot/stage1.asm
[bits 16]
[org 0x7c00]
    ; 0x7c00
    ; SEGMENT:OFFSET
    ; adres = SEGMENT * 0x10 + OFFSET
    ; CS 07c0  0000
    ; IP 0000  7c00
    ;          ****

    mov [BOOT_DRIVE], dl        ; BIOS stores our boot drive in DL

    mov bp, 0x9000              ; Set-up stack.
    mov sp, bp

    mov bx, MSG_LOAD_STAGE2
    call print_string

    call load_stage2

    ; Jump to stage 2
    jmp word 0x2000:0x0000

    jmp $

load_stage2:
    ; ES:BX = pointer to buffer
    mov ax, 0x2000 ; 0x2000:0x0000
    mov es, ax
    xor bx, bx

    mov dh, 1        ; Number of sectors. With the nop opcodes, this forms the
                     ; marker "\xb6\xXX\x90\x90" which could be used by some
                     ; external script to replace the exact size of stage2
    nop
    nop
    mov dl, [BOOT_DRIVE]
    call disk_load

    ret


%include "print_string.asm"
%include "disk_load.asm"

; Global variables
BOOT_DRIVE       db 0
MSG_LOAD_STAGE2  db "Loading stage2", 0xA, 0xD, 0


epilogue:
%if ($ - $$) > 510
%fatal "Bootloader code exceed 512 bytes."
%endif

times 510 - ($ - $$) db 0
db 0x55
db 0xAA
