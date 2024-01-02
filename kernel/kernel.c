#include "drivers/acpi.h"
#include "drivers/ioapic.h"
#include "drivers/kbd.h"
#include "drivers/screen.h"
#include "drivers/timer.h"
#include "drivers/uart.h"
#include "kernel/gdt.h"
#include "kernel/idt.h"
#include "kernel/kalloc.h"
#include "kernel/paging.h"
#include "kernel/pic.h"
#include "kernel/pmem.h"
#include "kernel/proc.h"
#include "kernel/spinlock.h"

extern char __k_start, __k_end; // defined in kernel.lds

void main(const struct pmem_info *mem_info) {
    consoleinit();
    print("-------->>KERNEL START<<--------\n");
    print("FOUDIL WAS HERE\n(c) 20222-2023\n");

    gdt_init();

    pmem_init(mem_info);
    cprintf("Max usable kernel memory address: 0x%p\n", phys_end - 1);

    idt_init();
    print("IDT initialized\n");

    pic_init();
    print("PIC initialized\n");

    timer_init();      // PIT timer support
    print("PIT Timer initialized\n");

    kbd_init();        // PS/2 keyboard support
    print("PS/2 keyboard initialized\n");

    uint32_t kend = PGROUNDUP(&__k_end);
    kinit1((void*)kend, P2V(4*1024*1024)); // phys page allocator
    paging_init();
    print("Pagination enabled\n");

    acpi_init();
    ioapicinit();    // another interrupt controller
    uartinit();      // serial port
    print("COM1 UART serial port enabled\n");

    sti();             // Enable interrupts. Now done by scheduler()

    kinit2(P2V(4*1024*1024), P2V(phys_end));
    print("Kernel heap allocator initialized\n");

    cpu_init();
    print("CPU state initialized\n");
    process_init();
    print("Process table ready\n");

    initproc_init();
    print("Init process created\n");

    while (1)   // CPU idles
        __asm__ __volatile__( "hlt" );
}
