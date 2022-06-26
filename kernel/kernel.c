#include "drivers/kbd.h"
#include "drivers/screen.h"
#include "drivers/timer.h"
#include "kernel/idt.h"
#include "kernel/pic.h"
#include "kernel/pmem.h"

void main(const struct pmem_info *pmem_info) {
    clear_screen();
    print("FOUDIL WAS HERE\n(c) 2022\n");

    pmem_init(pmem_info);

    idt_init();

    pic_init();

    timer_init();      // PIT timer support

    kbd_init();        // PS/2 keyboard support

    __asm__("sti");    // Enable interrupts
}
