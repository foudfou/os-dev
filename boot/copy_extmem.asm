; INT 15h, 87h (135) Move Block
;
; AH = 87h
; CX = number of words to copy (max 8000h)
; ES:SI -> global descriptor table (see #00499)
; Return:
; CF set on error
; CF clear if successful
; AH = status
;
; https://unix.stackexchange.com/questions/75732/real-mode-and-loading-linux-kernel-confusing/222770#222770
; https://github.com/Stefan20162016/linux-insides-code/blob/master/bootloader.asm
copy_extmem:
    push edx
    push es
    xor  ax, ax
    mov  es, ax
    mov  ah, 0x87
    mov  si, copy_extmem_gdt
    int  0x15
    jc   copy_extmem_error
    pop  es
    pop  edx
    ret

; Not the usual GDT format! http://www.ctyme.com/intr/rb-1527.htm
copy_extmem_gdt:
    times 16 db 0          ; zeros (used by BIOS)
copy_extmem_src:
    dw 0xffff                   ; segment limit
    dw KERNEL_OFFSET1 & 0xffff  ; first 2 bytes of source
    db KERNEL_OFFSET1 >> 16 ; 3rd/last byte of source address, so 0x20000 = 128 KiB
    db 0x93                 ; data access rights
    dw 0                    ;
copy_extmem_dst:
    dw 0xffff                  ; segment limit
    dw KERNEL_OFFSET2 & 0xffff ; 24-bit linear destination address, low byte first
    db KERNEL_OFFSET2 >> 16    ; load protected-mode kernel to 100000h
    db 0x93                    ; data access rights
    dw 0
    times 16 db 0

copy_extmem_error:
    mov bx, LOAD_ERROR_MSG
    call print_string

    jmp $


; Variables
LOAD_ERROR_MSG db "Load to himem failed", 0
