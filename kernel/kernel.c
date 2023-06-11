#include "drivers/kbd.h"
#include "drivers/screen.h"
#include "drivers/timer.h"
#include "kernel/gdt.h"
#include "kernel/idt.h"
#include "kernel/kalloc.h"
#include "kernel/paging.h"
#include "kernel/pic.h"
#include "kernel/pmem.h"

#include "lib/string.h"

extern char __k_start, __k_end; // defined by kernel.lds

void main(const struct pmem_info *mem_info) {
    clear_screen();
    print("FOUDIL WAS HERE\n(c) 2022\n");

    gdt_init();

    uint32_t phys_end = 0;
    pmem_init(mem_info, &phys_end);
    cprintf("Max usable memory address: 0x%p\n", phys_end - 1);

    idt_init();
    print("IDT initialized\n");

    pic_init();
    print("PIC initialized\n");

    timer_init();      // PIT timer support
    print("PIT Timer initialized\n");

    kbd_init();        // PS/2 keyboard support
    print("PS/2 keyboard initialized\n");

    __asm__("sti");    // Enable interrupts

    uint32_t kend = PGROUNDUP(&__k_end);
    kinit1((void*)kend, P2V(4*1024*1024)); // phys page allocator
    paging_init(phys_end);
    print("Pagination enabled\n");

    kinit2(P2V(4*1024*1024), P2V(phys_end));
    print("Kernel heap allocator initialized\n");

    while (1)   // CPU idles
        __asm__ __volatile__( "hlt" );
}
