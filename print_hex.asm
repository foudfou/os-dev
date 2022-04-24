;
; A routine that  prints an hex number.
;

; prints the value of DX as hex.
print_hex:
    ; TODO: manipulate chars at HEX_OUT to reflect DX

    ; pusha/popa considered expensive, but easier than using the stack for now
    ; https://stackoverflow.com/questions/70530930/usage-of-pusha-popa-in-function-prologue-epilogue
    pusha

    call convert_hex
    mov [HEX_OUT+2], ax         ; Write result of DH's conversion. Byte-order
                                ; will be reversed in memory (HEX_OUT)
    mov [HEX_OUT+4], cx         ; Write result of DL's conversion.

    mov bx, HEX_OUT    ; print the string pointed to
    call print_string  ; by BX

    popa
    ret

; IN: dx
; OUT: ax, cx
convert_hex:
    ; In little-endian architectures, registers and memory byte order are
    ; opposit. Since we'll later copy from reg to mem, we'll have to produce
    ; results in reverse-order.
    mov byte al, dl             ; Take first byte from source DX
    call convert_nibble         ; Convert lower nibble, result in AL
    mov byte ah, al             ; Save result for first nibble

    mov byte al, dl             ; Take first byte from source DX
    shr byte al, 4              ; Consider higher nibble
    call convert_nibble         ; Convert higher nibble, result in AL

    mov cx, ax                  ; Save result to CX

    ; Inlining (copy-paste) instead of using a loop is probably ok.
    mov byte al, dh             ; Take other byte from source DX
    call convert_nibble         ; Convert lower nibble, result in AL
    mov byte ah, al             ; Save result for first nibble

    mov byte al, dh             ; Take other byte from source DX
    shr byte al, 4              ; Consider higher nibble
    call convert_nibble         ; Convert higher nibble, result in AL

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
