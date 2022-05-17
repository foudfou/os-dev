print_string:
    pusha ; Push all register values to the stack

    ; We could inc bx directly
    mov si, 0

print_string_loop:
    mov al, byte [bx+si]
    cmp al, 0
    je print_string_done

    mov ah, 0x0e ; int =10/ ah =0x0e -> BIOS tele-type output
    int 0x10 ; print the character in al

    inc si
    jmp print_string_loop

print_string_done:
    popa ; Restore original register values
    ret
