[bits 32]

section .text

[extern hello]
[extern exit]

global start
start:
    push 1
    push 2
    push 3
    call sys1

end:
    call exit
    jmp end

sys1:
    push HELLO_STR
    push HELLO_STR
    push 123
    call hello
    add esp, 4*3                ; cleanup
    ret

HELLO_STR db "Hello wolrd!", 0
