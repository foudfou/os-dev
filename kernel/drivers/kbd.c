#include <stdbool.h>
#include "drivers/screen.h"
#include "idt.h"
#include "low_level.h"
#include "pic.h"

#include "drivers/kbd.h"

#define IO_PORT_KBD_DATA 0x60
#define IO_PORT_KBD_CTL  0x64

#define KBD_EXTENDED_SCANCODE 0xE0
#define KBD_IS_MAKECODE(c)    ((c) < KBD_BREAKCODE_LIMIT)
#define KBD_IS_BREAKCODE(c)   ((c) >= KBD_BREAKCODE_LIMIT)

#define BOOL_TOGGLE(a) (a) ? false : true

extern const int   kbd_scanmap_set1[KBD_BREAKCODE_LIMIT];
extern const char *kbd_scanmap_ascii_regular[KBD_BREAKCODE_LIMIT];
extern const char *kbd_scanmap_ascii_shift[KBD_BREAKCODE_LIMIT];

static bool shift, alt, ctrl;

/**
 * Keyboard interrupt handler registered for IRQ # 0.
 * Currently just prints a tick message.
 */
static void kbd_interrupt_handler(struct interrupt_state *state) {
    (void) state;   /** Unused. */

    int key = KEY_NULL;

    /**
     * Read our the event's scancode. Translate the scancode into a key
     * event, following the scancode set 1 mappings. Note mature implementations
     * are more cautious and take extra steps to check the state of the
     * keyboard using the keyboard status register (read 0x64).
     */
    uint8_t scancode = inb(IO_PORT_KBD_DATA);
    // Scancodes with extended byte (E0) generate two interrupts. Strictly
    // speaking we should thus record that we got an extended scancode when
    // reeiving E0 and wait for the following interrupt. I guess nowadays
    // there's no real need for waiting. So we pull the next scancode
    // immediately.
    if (scancode == KBD_EXTENDED_SCANCODE || scancode == 0xE1) {
        scancode = inb(IO_PORT_KBD_DATA);
    }

    // Interstingly bochs reads the Xorg key mapping. So `setxkbmap
    // -option shift:both_capslock_cancel` results in both Shift keys
    // generating capslock-release (BA). Qemu has its own key mapping.
    int idx = (KBD_IS_MAKECODE(scancode)) ? scancode : scancode - KBD_BREAKCODE_LIMIT;
    key = kbd_scanmap_set1[idx];

    switch (key) {
    case KEY_LCTRL:
    case KEY_RCTRL:
        ctrl = BOOL_TOGGLE(ctrl);
        break;

    case KEY_LSHIFT:
    case KEY_RSHIFT:
        shift = BOOL_TOGGLE(shift);
        break;

    case KEY_LALT:
    case KEY_RALT:
        alt = BOOL_TOGGLE(alt);
        break;

    default:
        if (key != KEY_NULL && KBD_IS_MAKECODE(scancode)) {
            const char *str = shift ?
                kbd_scanmap_ascii_shift[key] :
                kbd_scanmap_ascii_regular[key];
            print(str);
        }
    }
}

/** Initialize the PS/2 keyboard device. */
void kbd_init() {
    shift = alt = ctrl = false;

    isr_register(IDT_IRQ_BASE + IDT_IRQ_KEYBOARD, &kbd_interrupt_handler);

    pic_enable_irq_line(PIC_INT_KEYBOARD);
}
