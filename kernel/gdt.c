#include "drivers/screen.h"
#include "kernel/cpu.h"
#include "kernel/idt.h"
#include "lib/debug.h"

#include "kernel/gdt.h"

extern void gdt_load(struct gdtr*);

static void protection_fault_handler(struct interrupt_state *state) {
    uint32_t selector = state->err_code;
    warn("Caught protection fault {\n"
         "  eip = 0x%x\n"
         "  external: %d\n"
         "  table:    %d\n" // 0=GDT, 1=IDT, 2=LDT, 3=IDT
         "  index:    %d\n"
         "}", state->eip, selector & 0b1, (selector >> 1) & 0x3, (selector >> 3) & 0x1FFF);
    panic("protection fault not handled!");
}

// From WASOS: «– Granularity: 1, if set, this multiplies our limit by 4 K
// (i.e. 16*16*16), so our 0xfffff would become 0xfffff000 (i.e. shift 3 hex
// digits to the left), allowing our segment to span 4 Gb of memory». This is
// why limit is shifted by 12 and then by 28.
void
gdt_set_entry(struct segdesc *seg, uint32_t base, uint32_t limit,
              uint8_t access, uint8_t flags) {
    seg->limit_lo = ((flags & SEG_4K) ? (limit >> 12) : (limit)) & 0xffff;
    seg->base_lo  = base & 0xffff;
    seg->base_mi  = (base >> 16) & 0xff;
    seg->access   = access;
    seg->flags    = ((flags & SEG_4K) ? (limit >> 28) : (limit >> 16)) | (flags & 0xf0);
    seg->base_hi  = base >> 24;
}


void gdt_init(void) {
    isr_register(IDT_INT_GPFLT, &protection_fault_handler);

    struct cpu *c = &cpus[0]; //cpuid()]; FIXME when we do smp
    gdt_set_entry(&c->gdt[SEG_KCODE], 0, 0xffffffff, SEG_APP|SEG_CODE|SEG_RING0|SEG_RW|SEG_PRESENT, SEG_32BIT|SEG_4K);
    gdt_set_entry(&c->gdt[SEG_KDATA], 0, 0xffffffff, SEG_APP|SEG_RING0|SEG_RW|SEG_PRESENT, SEG_32BIT|SEG_4K);
    gdt_set_entry(&c->gdt[SEG_UCODE], 0, 0xffffffff, SEG_APP|SEG_CODE|SEG_RING3|SEG_RW|SEG_PRESENT, SEG_32BIT|SEG_4K);
    gdt_set_entry(&c->gdt[SEG_UDATA], 0, 0xffffffff, SEG_APP|SEG_RING3|SEG_RW|SEG_PRESENT, SEG_32BIT|SEG_4K);

    /* __asm__ __volatile__("xchg %bx, %bx"); // Bochs magic break */

    struct gdtr gdtr = {.limit = sizeof(c->gdt), .base = (uint32_t)&c->gdt};
    gdt_load(&gdtr);
}
