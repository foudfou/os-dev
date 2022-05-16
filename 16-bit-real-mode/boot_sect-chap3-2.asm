    ;
    ; A simple boot sector program that demonstrates addressing.
    ;

; ; Question 1: With label references correction, attempt1 still fails (wrong
; ; value), attempt2 succeeds (well corrected), attempt3 fails (double
; ; correction), attempt4 succeeds (absolute addressing).
; [org 0x7c00]

    mov ah, 0x0e ; int 10/ ah = 0 eh -> scrolling teletype BIOS routine

    ; First attempt - doesn't work as al contains the label *offset* = 0x1d
    ; (29) bytes from the start of our boot sector. NOTE original book mentions
    ; 0x1e, but nasm produces a different bin.
    mov al, the_secret
    int 0x10 ; Does not print an X

    ; Second attempt - doesn't work as al contains the relative *offset* which
    ; points to a wrong absolute memory area.
    mov al, [the_secret]
    int 0x10 ; Does not print an X

    ; Third attempt
    mov bx, the_secret
    add bx, 0x7c00              ; BIOS always loads the boot sector boot sector
                                ; at 0x7c00.
    mov al, [bx]
    int 0x10 ; Does print an X!

    ; Fourth attempt
    mov al, [0x7c1d]
    int 0x10 ; Does print an X!

    jmp $ ; Jump forever.

the_secret:
    db "X"

    ; Padding and magic BIOS number.
    times 510-($-$$) db 0
    dw 0xaa55
