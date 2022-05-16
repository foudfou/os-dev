; mov bx, 30
; if (bx <= 4) {
;   mov al, 'A'
; } else if (bx < 40) {
;   mov al, 'B'
; } else {
;   mov al, 'C'
; }

    mov bx, 300

    cmp bx, 4
    jle if_le_4
    cmp bx, 40
    jl if_l_40
    mov al, 'C'
    jmp the_end

if_le_4:
    mov al, 'A'
    jmp the_end

if_l_40:
    mov al, 'B'

the_end:
    mov ah, 0x0e         ; int 10/ ah = 0 eh -> scrolling teletype BIOS routine
    int 0x10             ; print the character in al

    jmp $

; Padding and magic number.
times 510-($-$$) db 0
dw 0xaa55
