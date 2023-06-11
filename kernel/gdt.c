#include "lib/debug.h"
#include "kernel/idt.h"

#include "kernel/gdt.h"

extern void gdt_load(struct gdtr*);

static struct segdesc gdt[NSEGS] = {0}; // x86 global descriptor table

// From WASOS: «– Granularity: 1, if set, this multiplies our limit by 4 K
// (i.e. 16*16*16), so our 0xfffff would become 0xfffff000 (i.e. shift 3 hex
// digits to the left), allowing our segment to span 4 Gb of memory». This is
// why lim is shifted by 12 and then by 28.
static void
gdt_set_entry(int idx, uint32_t base, uint32_t limit,
              uint8_t access, uint8_t flags) {
    gdt[idx].limit_lo = (limit >> 12) & 0xffff;
    gdt[idx].base_lo  = base & 0xffff;
    gdt[idx].base_mi  = (base >> 16) & 0xff;
    gdt[idx].access   = access;
    gdt[idx].flags    = limit >> 28 | (flags & 0xf0);
    gdt[idx].base_hi  = base >> 24;
}

static void protection_fault_handler(struct interrupt_state *state) {
    (void) state;   /** Unused. */

    panic("protection fault not handled!");
}


void gdt_init(void) {
    isr_register(INT_IRQ_PROT_FAULT, &protection_fault_handler);

    gdt[0] = (struct segdesc){0};
    gdt_set_entry(SEG_KCODE, 0, 0xffffffff, SEG_CODE|SEG_RING0|SEG_PREAD, SEG_32BIT|SEG_4K);
    gdt_set_entry(SEG_KDATA, 0, 0xffffffff, SEG_RING0|SEG_PREAD, SEG_32BIT|SEG_4K);
    gdt_set_entry(SEG_UCODE, 0, 0xffffffff, SEG_CODE|SEG_RING3|SEG_PREAD, SEG_32BIT|SEG_4K);
    gdt_set_entry(SEG_UDATA, 0, 0xffffffff, SEG_RING3|SEG_PREAD, SEG_32BIT|SEG_4K);

    /* __asm__ __volatile__("xchg %bx, %bx"); // Bochs magic break */

    struct gdtr gdtr = {.limit = sizeof(gdt), .base = (uint32_t)&gdt};
    gdt_load(&gdtr);
}
