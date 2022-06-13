#include "drivers/kbd.h"
#include "drivers/screen.h"
#include "drivers/timer.h"
#include "kernel/idt.h"
#include "kernel/pic.h"

void main () {
    clear_screen();
    print("FOUDIL WAS HERE\n(c) 2022\n");

    idt_init();

    pic_init();

    timer_init();   // PS/2 keyboard support

    keyboard_init();   // PS/2 keyboard support

    __asm__("sti");    // Enable interrupts
}
