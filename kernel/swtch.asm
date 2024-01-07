; Context switch
;
;   void swtch(struct context **old, struct context *new);
;
; Save the current registers on the stack, creating
; a struct context, and save its address in *old.
; Switch stacks to new and pop previously-saved registers.

global swtch
swtch:
    mov eax, [esp+4]            ; old
    mov edx, [esp+8]            ; new

    ; Save old callee-saved registers
    push ebp
    push ebx
    push esi
    push edi

    ; Switch stacks
    mov [eax], esp
    mov esp, edx

    ; Load new callee-saved registers
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret
