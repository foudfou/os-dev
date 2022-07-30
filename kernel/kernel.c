#include "drivers/kbd.h"
#include "drivers/screen.h"
#include "drivers/timer.h"
#include "kernel/idt.h"
#include "kernel/paging.h"
#include "kernel/pic.h"
#include "kernel/pmem.h"

void main(const struct pmem_info *pmem_info) {
    clear_screen();
    print("FOUDIL WAS HERE\n(c) 2022\n");

    uint64_t ram_size = 0;
    pmem_init(pmem_info, &ram_size);
    cprintf("Available memory: %l bytes\n", ram_size);

    idt_init();
    print("IDT initialized\n");

    pic_init();
    print("PIC initialized\n");

    timer_init();      // PIT timer support
    print("PIT Timer initialized\n");

    kbd_init();        // PS/2 keyboard support
    print("PS/2 keyboard initialized\n");

    paging_init(ram_size);
    print("Pagination enabled\n");

    __asm__("sti");    // Enable interrupts

    // TESTING page fault
    uint32_t faulty_addr = 0xBCDE0123; // above 128MiB phys mem!
    int dummy = *((int *) faulty_addr);
    cprintf("%p\n", dummy);
}
