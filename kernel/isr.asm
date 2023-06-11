[bits 32] ; protected mode

section .text

[extern isr_handler]

global idt_load
idt_load:
    push ebp
    mov ebp,esp
    push eax
    mov eax,[ebp+8]
    lidt [eax]
    pop eax
    pop ebp
    ret

%macro isr_err_stub 1
isr_stub_%+%1:
    ; the error code is already pushed
    push %1                    ; push interrupt number
    ;
    ; DO NOT MAKE THE MISTAKE OF COPY-PASTING OSDEV! As this would call `iret`
    ; twice, resulting in bochs yelling `not a valid code segment !`
    ;
    ;  call isr_wrapper
    ;  iret
    ;
    jmp isr_wrapper
%endmacro

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push 0                     ; push fake error code
    push %1                    ; push interrupt number
    jmp isr_wrapper
%endmacro

isr_wrapper:
    ; Save the registers. We'll use them as arguments to the handler. See
    ; interrupt_state. Note at this stage the stack already has got: eflags,
    ; cs, eip, error_code. PUSHAD saves EDI, ESI, EBP, ESP, EBX, EDX, ECX,
    ; EAX.
    pushad

    ; Save DS (as lower 16 bits).
    mov ax, ds
    push eax

    ; Push a pointer to the current stuff on stack, which are:
    ;   - DS
    ;   - EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    ;   - Interrupt number, Error code
    ;   - EIP, CS, EFLAGS, User's ESP, SS (these are previously pushed
    ;     by the processor when entering this interrupt)
    ;
    ; This pointer is the argument of our `isr_handler` function.
    mov eax, esp
    push eax

    ; Load kernel mode data segment descriptor.
    mov ax, 0x10                ; FIXME how to share asm-defined CODE_SEG ?
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; CLears Direction flag. C code following the sysV ABI requires DF to be
    ; clear on function.
    cld

    call isr_handler

    add esp, 4                  ; Clean up the pointer argument.

    ; Restore previous segment descriptor.
    pop eax
    mov ax, ds
    mov ax, es
    mov ax, fs
    mov ax, gs

    ; Restores EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX.
    popad

    add esp, 8                  ; Cleans up error code and ISR number.

    iret                        ; This pops CS, EIP, EFLAGS, SS, and ESP.

global null_handler
null_handler:
    iret

; Exceptions
isr_no_err_stub  0
isr_no_err_stub  1
isr_no_err_stub  2
isr_no_err_stub  3
isr_no_err_stub  4
isr_no_err_stub  5
isr_no_err_stub  6
isr_no_err_stub  7
isr_err_stub     8
isr_no_err_stub  9
isr_err_stub     10
isr_err_stub     11
isr_err_stub     12
isr_err_stub     13
isr_err_stub     14
isr_no_err_stub  15
isr_no_err_stub  16
isr_err_stub     17
isr_no_err_stub  18
isr_no_err_stub  19
isr_no_err_stub  20
isr_no_err_stub  21
isr_no_err_stub  22
isr_no_err_stub  23
isr_no_err_stub  24
isr_no_err_stub  25
isr_no_err_stub  26
isr_no_err_stub  27
isr_no_err_stub  28
isr_no_err_stub  29
isr_err_stub     30
isr_no_err_stub  31

; Interrupts
isr_no_err_stub  32
isr_no_err_stub  33
isr_no_err_stub  34
isr_no_err_stub  35
isr_no_err_stub  36
isr_no_err_stub  37
isr_no_err_stub  38
isr_no_err_stub  39
isr_no_err_stub  40
isr_no_err_stub  41
isr_no_err_stub  42
isr_no_err_stub  43
isr_no_err_stub  44
isr_no_err_stub  45
isr_no_err_stub  46
isr_no_err_stub  47


section .rodata:

global isr_stub_table
isr_stub_table:
%assign i 0
%rep    48
    dd isr_stub_%+i
%assign i i+1
%endrep
