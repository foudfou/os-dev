[bits 16]
[org 0x0000]

start16:
    mov ax, 0x2000       ; Update segment registers
    mov ds, ax
    mov es, ax

    mov ax, 0x1f00       ; Setup stack.
    mov ss, ax           ; We create a stack, ss is a seg register on the stack
    xor sp, sp           ; the stack will grow towards the zero address, our code in the second

    mov bx, MSG_START_STAGE2
    call print_string

    jmp $                  ; Hang.

%include "print_string.asm"

MSG_START_STAGE2 db "Starting stage2", 0xA, 0xD, 0

times (512 - ($ - $$) % 512) db 0
