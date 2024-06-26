; Ensures that we jump straight into the kernel's entry function.
[bits 32]     ; We're in protected mode by now , so use 32-bit instructions.

section .text

; NASM's directives come in two types: user-level directives and primitive
; directives. Typically, each directive has a user-level form and a primitive
; form. In almost all cases, we recommend that users use the user-level forms
; of the directives, which are implemented as macros which call the primitive
; forms.  Primitive directives are enclosed in *square brackets*; user-level
; directives are not.
[extern main] ; Declate that we will be referencing the external symbol 'main',
              ; so the linker can substitute the final address

%include "boot/defs.asm"
%include "kernel/paging_defs.asm"

KSTACKSIZE equ 0x4000           ; 16k

global _start ; As a convention, we'll use '_start' to label
_start:       ; the entry point and use that in the linker script.

entry:
    ; Turn on page size extension for 4Mbyte pages
    mov    eax, cr4
    or     eax, CR4_PSE
    mov    cr4, eax

    ; Set page directory
    mov    eax, entrypgdir - KERNBASE
    mov    cr3, eax

    ; Turn on paging.
    mov    eax, cr0
    or     eax, CR0_PG|CR0_WP
    mov    cr0, eax

    ;; Setup kernel stack, distinct from the bootloader (switch_to_pm.asm)
    ;; https://ordoflammae.github.io/littleosbook/#setting-up-a-stack
    mov esp, stack + KSTACKSIZE ; point esp to the start of the
                                ; stack (end of memory area)
    ; xchg bx, bx                 ; Bochs magic break

    ; We could reload the GDT here with a new virtual address, as in
    ; https://github.com/davidedellagiustina/scratch-os/blob/master/src/kernel/kernel_entry.asm#L22,
    ; but it's ok to do this later in the C part of the kernel.

    mov eax, E820_MAP + KERNBASE ; Pass physical memory map as argument to
    push eax                     ; kernel's main function

    ; make eip point to a virtual address in the higher half
    mov eax, main
    call eax  ; Absolute Jump. Using `call` instead of `jmp` as we want to pass
              ; arguments to main().

    jmp $     ; Hang forever when we return from the kernel


section .data

NKERNPG     equ (KERNBASE >> 22)  ; Page directory index of kernel's 4MB PTE
NPDENTRIES  equ 1024              ; Number directory entries per page directory

; https://github.com/amanuel2/OS_Mirror/blob/master/boot.asm
; Equivalent of xv6's entrypgdir[] https://github.com/mit-pdos/xv6-public/blob/eeb7b415dbcb12cc362d0783e41c3d1f44066b17/main.c#L103
align 0x1000
entrypgdir:
    ; This page directory entry identity-maps the first 4MB of the 32-bit physical address space.
    ; All bits are clear except the following:
    ; bit 7: PS The kernel page is 4MB.
    ; bit 1: RW The kernel page is read/write.
    ; bit 0: P  The kernel page is present.
    ; This entry must be here -- otherwise the kernel will crash immediately after paging is
    ; enabled because it can't fetch the next instruction! It's ok to unmap this page later.
    dd 0x00000083
    times (NKERNPG - 1) dd 0                 ; Pages before kernel space.
    ; This page directory entry defines a 4MB page containing the kernel.
    dd 0x00000083
    times (NPDENTRIES - NKERNPG - 1) dd 0  ; Pages after the kernel image.


section .bss
align 4                         ; align at 4 bytes
stack:                          ; label points to beginning of memory
    resb KSTACKSIZE             ; reserve stack for the kernel
