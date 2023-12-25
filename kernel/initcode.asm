global start
start:

    ; Prints a letter 'H' in the second to last VGA line, just to show
    ; that init gets executed. Please ignore the dirty numbers.
    mov eax, 0xB8000
    mov bx, 0x0F48
    mov [eax+3720], bx

    ; Infinite halt loop trick.
    cli
halt:
    hlt
    jmp halt
