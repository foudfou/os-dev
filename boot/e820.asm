[bits 16]
; Lifted from https://wiki.osdev.org/Detecting_Memory_(x86)#Code_Examples#Getting_an_E820_Memory_Map
; use the INT 0x15, eax= 0xE820 BIOS function to get a memory map
; note: initially di is 0, be sure to set it to a value so that the BIOS code will not be overwritten.
;       The consequence of overwriting the BIOS code will lead to problems like getting stuck in `int 0x15`
; inputs: es:di -> destination buffer for 24 byte entries
; outputs: bp = entry count, trashes all registers except esi
e820_start:
    mov di, MEM_MAP+4 ; Set di to 0x8004. Otherwise this code will get stuck in `int 0x15` after some entries are fetched
    xor ebx, ebx   ; ebx must be 0 to start
    xor bp, bp     ; keep an entry count in bp
    mov edx, 0x534D4150         ; Place "SMAP" into edx
    mov eax, 0xe820
    mov [es:di + 20], dword 1   ; force a valid ACPI 3.X entry
    mov ecx, 24                 ; ask for 24 bytes
    int 0x15
    jc short e820_failed ; carry set on first call means "unsupported function"
    mov edx, 0x534D4150  ; Some BIOSes apparently trash this register?
    cmp eax, edx         ; on success, eax must have been reset to "SMAP"
    jne short e820_failed
    test ebx, ebx       ; ebx = 0 implies list is only 1 entry long (worthless)
    je short e820_failed
    jmp short e820_jmpin
e820_lp:
    mov eax, 0xe820             ; eax, ecx get trashed on every int 0x15 call
    mov [es:di + 20], dword 1   ; force a valid ACPI 3.X entry
    mov ecx, 24                 ; ask for 24 bytes again
    int 0x15
    jc short e820_f         ; carry set means "end of list already reached"
    mov edx, 0x534D4150         ; repair potentially trashed register
e820_jmpin:
    jcxz e820_skipent           ; skip any 0 length entries
    cmp cl, 20                  ; got a 24 byte ACPI 3.X response?
    jbe short e820_notext
    test byte [es:di + 20], 1   ; if so: is the "ignore this data" bit clear?
    je short e820_skipent
e820_notext:
    mov ecx, [es:di + 8] ; get lower uint32_t of memory region length
    or ecx, [es:di + 12] ; "or" it with upper uint32_t to test for zero
    jz e820_skipent      ; if length uint64_t is 0, skip entry
    inc bp               ; got a good entry: ++count, move to next storage spot
    add di, 24
e820_skipent:
    test ebx, ebx               ; if ebx resets to 0, list is complete
    jne short e820_lp
e820_f:
    mov [MEM_MAP], bp          ; store the entry count
    clc ; there is "jc" on end of list to this point, so the carry must be cleared
    ret
e820_failed:
    stc                         ; "function unsupported" error exit
    ret
