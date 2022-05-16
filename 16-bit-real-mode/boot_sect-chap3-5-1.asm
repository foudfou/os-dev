;
; A boot sector that prints an hex number.
;
[org 0x7c00]    ; Tell the assembler where this code will be loaded

    mov dx, 0x1fb6    ; store the value to print in dx
    call print_hex    ; call the function

    jmp $                    ; Hang

; ORDER OF INLCUDES AND CODE IS IMPORTANT!
%include "print_hex.asm"
%include "print_string.asm"

    ; Padding  and  magic  number.
    times  510-($-$$) db 0
    dw 0xaa55

; In bochs, inspect/debug mem with: x /40hx 0x7c00
