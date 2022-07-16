; GDT - here we use the simplest workable configuration of segment registers is
; described by Intel as the "basic flat model", whereby two *overlapping*
; segments are defined that cover the full 4 GB of addressable memory, one for
; code and the other for data.
;
; The fact that in this model these two segments overlap means that there is no
; attempt to protect one segment from the other, nor is there any attempt to
; use the paging features for virtual memory. It pays to keep things simple
; early on, especially since later we may alter the segment descriptors more
; easily once we have booted into a higher-level language.
gdt_start:

gdt_null:   ; the mandatory null descriptor
    dd 0x0  ; 'dd' means define double word (i.e. 4 bytes)
    dd 0x0

; The code segment spans 0-0xfffff and we set the Granularity flag to 1, which
; indicates a Limit scale of 4 KiB blocks. We can thus address 0-fffff00 (4GB).
gdt_code:   ; the code segment descriptor
    ; base=0x0, limit=0xfffff,
    ; 1st flags: (present)1 (privilege)00 (descriptor type)1 -> 1001b
    ; type flags: (code)1 (conforming)0 (readable)1 (accessed)0 -> 1010b
    ; 2nd flags: (granularity)1 (32-bit default)1 (64-bit seg)0 (AVL)0 -> 1100b
    dw 0xffff     ; Limit (bits 0-15)
    dw 0x0000     ; Base (bits 0-15)
    db 0x00       ; Base (bits 16-23)
    ; 0x9A
    db 10011010b  ; 1st flags, type flags
    ; 0xCF
    db 11001111b  ; 2nd flags, Limit (bits 16-19)
    db 0x00       ; Base (bits 24-31)

gdt_data:         ; the data segment descriptor
    ; Same as code segment except for the type flags:
    ; type flags: (code)0 (expand down)0 (writable)1 (accessed)0 -> 0010b
    dw 0xffff     ; Limit (bits 0-15)
    dw 0x0000     ; Base (bits 0-15)
    db 0x00       ; Base (bits 16 -23)
    ; 0x92
    db 10010010b  ; 1st flags, type flags
    ; 0xCF
    db 11001111b  ; 2nd flags, Limit (bits 16-19)
    db 0x00       ; Base (bits 24 -31)

gdt_end:          ; The reason for putting a label at the end of the
                  ; GDT is so we can have the assembler calculate
                  ; the size of the GDT for the GDT decriptor (below)

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size of our GDT, always less one
                                ; of the true size
    dd gdt_start                ; Start address of our GDT

; Define some handy constants for the GDT segment descriptor offsets, which
; are what segment registers must contain when in protected mode. For example,
; when we set DS = 0x10 in PM, the CPU knows that we mean it to use the
; segment described at offset 0x10 (i.e. 16 bytes) in our GDT, which in our
; case is the DATA segment (0x0 -> NULL; 0x08 -> CODE; 0x10 -> DATA)
;
; `equ` is basically a preprocessor #define. Can be accessed in C with `extern`
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
