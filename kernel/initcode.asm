global start
start:
    ; system call #1
    mov eax, 1
    int 0x40

    hlt
