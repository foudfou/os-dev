; TODO include C headers defines
; http://thomasloven.com/blog/2012/06/C-Headers-In-Asm/
IDT_TRAP_SYSCALL equ 64

SYS_hello equ 1

global start
start:
    push HELLO_STR
    push HELLO_STR
    push 123
    call hello

    hlt

hello:
    mov eax, SYS_hello
    int IDT_TRAP_SYSCALL
    ret

HELLO_STR db "Hello wolrd!", 0
