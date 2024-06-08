%include "kernel/idt_defs.asm"
%include "kernel/syscall_defs.asm"

%macro SYSCALL 1
global %1
%1:
    mov eax, SYS_%+%1
    int IDT_TRAP_SYSCALL
    ret
%endmacro

SYSCALL hello
SYSCALL exit

; For CFLAG += -fstack-protector without lib/debug.h
global __stack_chk_guard
__stack_chk_guard equ 0xBAAAAAAD

global __stack_chk_fail
__stack_chk_fail:
    hlt
