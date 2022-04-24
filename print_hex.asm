;
; A routine that  prints an hex number.
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
    mov bx, HEX_OUT
    add bx, 2

    ; Best to work with words, not bytes!
    mov ax, dx
    and ax, 0xf000
    shr ax, 12
    call convert_nibble
    mov [bx], al
    inc bx

    mov ax, dx
    and ax, 0x0f00
    shr ax, 8
    call convert_nibble
    mov [bx], al
    inc bx

    mov ax, dx
    and ax, 0x00f0
    shr ax, 4
    call convert_nibble
    mov [bx], al
    inc bx

    mov ax, dx
    and ax, 0x000f
    call convert_nibble
    mov [bx], al

    mov bx, HEX_OUT    ; print the string pointed to
    call print_string  ; by BX

    popa
    ret


; Convert lower nibble in AL to ascii code
convert_nibble:
    and byte al, 0x0f           ; Work on lower nibble only.
    cmp byte al, 0xa
    jl digit
letter:
    add byte al, 'a' - 10       ; We want: 0xa <-> index 0 <-> 'a'.
    ret
digit:
    add byte al, '0'
    ret


    ; global  variables
HEX_OUT: db '0x0000', 0

%include "print_string.asm"
