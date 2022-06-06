; Ensures that we jump straight into the kernel's entry function.
[bits 32]     ; We're in protected mode by now , so use 32-bit instructions.
[extern main] ; Declate that we will be referencing the external symbol 'main',
              ; so the linker can substitute the final address

global _start ; As a convention, we'll use '_start' to label
_start:       ; the entry point and use that in the linker script.
    call main ; Invoke main() in our C kernel
    jmp $     ; Hang forever when we return from the kernel
