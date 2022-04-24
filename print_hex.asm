; Look at git history to see previous attempts. Especially writing the
; conversion results to registers then to memory (HEX_OUT) raises endianness
; issues. In little-endian architectures, register and memory byte order are
; opposit. Copying between register and memory reverses bytes.

; prints the value of DX as hex.
print_hex:
    ; pusha/popa considered expensive, but easier than using the stack for now
    ; https://stackoverflow.com/questions/70530930/usage-of-pusha-popa-in-function-prologue-epilogue
    pusha

    ; It's not syntactically obvious how to write bytes directly to mem
    ; HEX_OUT. The trick is to store HEX_OUT into a register then use that reg
    ; as a pointer! Thx
    ; https://github.com/tobiasorama/exercises-and-files-from-writing-a-simple-OS-from-scratch/blob/main/print_hex.asm
    mov di, HEX_OUT
    add di, 2

    ; Another trick is to use an ALPHABET instead of a function to convert a
    ; nibble. Thx
    ; https://github.com/t-hanf/WASOS/blob/main/Chapter_3/utils.asm
print_hex_loop:
    cmp di, HEX_OUT+5
    jg print_hex_end

    ; Only BX can be used as an index register! https://stackoverflow.com/a/1797782
    mov bx, dx
    and bx, 0xf000
    shr bx, 12
    mov cx, [ALPHABET+bx]
    mov [di], cl

    inc di
    shl dx, 4                   ; Ok to modify dx as all regs are pushed already.
    jmp print_hex_loop

print_hex_end:
    mov bx, HEX_OUT    ; print the string pointed to
    call print_string  ; by BX

    popa
    ret


; global  variables
HEX_OUT: db '0x0000', 0

ALPHABET: db '0123456789abcdef'

%include "print_string.asm"
