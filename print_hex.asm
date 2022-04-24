;
; A routine that  prints an hex number.
;

; prints the value of DX as hex.
print_hex:
    ; TODO: manipulate chars at HEX_OUT to reflect DX

    ; pusha popa considered expensive, but easier for now
    ; https://stackoverflow.com/questions/70530930/usage-of-pusha-popa-in-function-prologue-epilogue
    pusha

    ; TODO how do we pass and manipulate HEX_OUT ?
    call set_byte

    mov bx, HEX_OUT    ; print the string pointed to
    call print_string  ; by BX

    popa
    ret

set_byte:
    ; Take dx first byte: 0x1f.
    ; Store it into HEX_OUT byte 3, after '0x'.
    ; NOTE NASM Requires Square Brackets For Memory References!
    mov [HEX_OUT+2], dh

    ; convert high byte half to ascii code
    ; 0x0 hex → 0x30 ascii, 0xa → 0x61
    shr byte [HEX_OUT+2], 4
    add byte [HEX_OUT+2], '0'

    ; Take dx first byte: 0x1f.
    ; Store it into HEX_OUT byte 4, after '0xX'.
    mov [HEX_OUT+3], dh

    ; convert low byte half to ascii code
    and byte [HEX_OUT+3], 0x0f
    add byte [HEX_OUT+3], 'a' - 10

    ; TODO if b < 0xa then add '0' else add 'a'-10

    ret

    ; global  variables
HEX_OUT: db '0x0000', 0

%include "print_string.asm"
