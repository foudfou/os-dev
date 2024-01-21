#include <stddef.h>
#include "drivers/screen.h"
#include "kernel/gdt.h"
#include "kernel/pic.h"

#include "kernel/idt.h"

extern uint32_t isr_stub_table[];

extern void idt_load(uint32_t idtr_ptr);

/**
 * The IDT table. Should contain 256 gate entries.
 *   -  0 -  31: reserved by x86 CPU for various exceptions
 *   - 32 - 255: free for our OS kernel to define
 *
 * Aligned for performance.
 */
__attribute__((aligned(0x10)))
static struct idt_entry idt[IDT_SIZE_MAX] = {0};

/** IDTR address register. */
static struct idtr idtr;

/** Table of ISRs. Unregistered entries MUST be NULL. */
isr_fn isr_table[IDT_SIZE_MAX] = {NULL};

static void
idt_set_descriptor(int idx, uint32_t base, uint16_t selector, uint8_t flags)
{
    idt[idx].base_lo   = (uint16_t) (base & 0xffff);
    idt[idx].selector  = (uint16_t) selector;
    idt[idx].zero      = (uint8_t) 0;
    idt[idx].type_attr = (uint8_t) flags;
    idt[idx].base_hi   = (uint16_t) ((base >> 16) & 0xFFFF);
}

/** Exposed to other parts for them to register ISRs. */
inline void isr_register(uint8_t int_no, isr_fn handler) {
    if (isr_table[int_no] != NULL) {
        cprintf("isr: handler already registered for interrupt # %d", int_no);
        return;
    }
    isr_table[int_no] = handler;
}

/**
 * ISR handler written in C.
 *
 * Receives a pointer to a structure of interrupt state. Handles the
 * interrupt and simply returns. Can modify interrupt state through
 * this pointer if necesary.
 */
void isr_handler(struct interrupt_state *state) {
    uint8_t int_no = state->int_no;

    if (isr_table[int_no] == NULL) {
        cprintf("missing handler for ISR interrupt # %d\n", int_no);
        panic("isr_handler");
    } else
        isr_table[int_no](state);

    if (int_no < IDT_IRQ_SIZE_MAX) { // interrupt
        uint8_t irq_no = state->int_no - IDT_IRQ_BASE;
        pic_send_eoi(irq_no); // ACK
    }
}

/**
 * Initialize the interrupt descriptor table (IDT) by setting up gate
 * entries of IDT, setting the IDTR register to point to our IDT address,
 * and then (through assembly `lidt` instruction) load our IDT.
 */
void idt_init() {
    // Unused entries and field all default to 0, as per previous initialization.
    for (uint8_t vector = 0; vector < IDT_IRQ_BASE; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], SEG_KCODE << 3, IDT_DESCRIPTOR_EXCEPTION);
    }

    for (uint8_t vector = IDT_IRQ_BASE; vector < IDT_IRQ_SIZE_MAX; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], SEG_KCODE << 3, IDT_DESCRIPTOR_EXTERNAL);
    }

    idt_set_descriptor(IDT_TRAP_SYSCALL, isr_stub_table[IDT_TRAP_SYSCALL], SEG_KCODE << 3, IDT_DESCRIPTOR_CALL);

    // Setup the IDTR register value.
    idtr.base  = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(struct idt_entry) * IDT_SIZE_MAX - 1;

    // Load the IDT.
    idt_load((uint32_t) &idtr);
}
