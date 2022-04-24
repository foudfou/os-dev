;
; A routine that prints an hex number.
;

; prints the value of DX as hex.
print_hex:
    ; TODO: manipulate chars at HEX_OUT to reflect DX

    ; pusha/popa considered expensive, but easier than using the stack for now
    ; https://stackoverflow.com/questions/70530930/usage-of-pusha-popa-in-function-prologue-epilogue
    pusha

    ; It's not syntactically obvious how to write bytes directly to mem
    ; HEX_OUT. The trick is to store HEX_OUT into a register then use that reg
    ; as a pointer! Thx
    ; https://github.com/tobiasorama/exercises-and-files-from-writing-a-simple-OS-from-scratch/blob/main/print_hex.asm
    mov di, HEX_OUT
    add di, 2                   ; Start writing from then end

    ; Another trick is to use an ALPHABET instead of a function to convert a
    ; nibble. Thx
    ; https://github.com/t-hanf/WASOS/blob/main/Chapter_3/utils.asm
    mov bx, dx
    and bx, 0xf000
    shr bx, 12
    add bx, ALPHABET
    mov al, [bx]
    mov [di], al
    inc di

    ; TODO can we avoid inlining (copy-paste)?
    mov bx, dx
    and bx, 0x0f00
    shr bx, 8
    add bx, ALPHABET
    mov al, [bx]
    mov [di], al
    inc di

    mov bx, dx
    and bx, 0x00f0
    shr bx, 4
    add bx, ALPHABET
    mov al, [bx]
    mov [di], al
    inc di

    mov bx, dx
    and bx, 0x000f
    add bx, ALPHABET
    mov al, [bx]
    mov [di], al

    mov bx, HEX_OUT    ; print the string pointed to
    call print_string  ; by BX

    popa
    ret


; global  variables
HEX_OUT: db '0x0000', 0

ALPHABET: db '0123456789abcdef'

%include "print_string.asm"
