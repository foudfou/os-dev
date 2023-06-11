; https://segmentfault.com/a/1190000040187304/en
[bits 32]

section .text

; «Whatever you do with the GDT has no effect on the CPU until you load new
; Segment Selectors into Segment Registers. For most of these registers, the
; process is as simple as using MOV instructions, but changing the CS register
; requires code resembling a jump or call to elsewhere, as this is the only way
; its value is meant to be changed.»
; https://wiki.osdev.org/GDT_Tutorial#Reload_Segment_Registers
global gdt_load
gdt_load:
    mov eax, [esp + 4]
    lgdt [eax]

    ; segment selector are in the form: [15-3: GDT Idx, 2: LDT(1)/GDT(0), 1-0:
    ; Priv]. Hence DS is 0b10000/0x10, and CS would be 0b1000/0x8.
    ; https://stackoverflow.com/a/23979175/421846
    mov ax, 0x10                ; KDATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov ss, ax

    mov ax, 0x18                ; UCODE
    mov gs, ax

    ; Actually reload the CS register.
    jmp 0x08:.flush             ; 0x08 is a stand-in for your code segment
.flush:
    ret
