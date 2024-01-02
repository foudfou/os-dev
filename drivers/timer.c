#include <stdint.h>
#include "drivers/screen.h"
#include "kernel/idt.h"
#include "kernel/low_level.h"
#include "kernel/pic.h"
#include "lib/debug.h"

#include "drivers/timer.h"

#define IO_PORT_TIMER_DATA0 0x40
#define IO_PORT_TIMER_CMD   0x43

static volatile uint64_t ticks;

/**
 * Timer interrupt handler registered for IRQ # 0.
 * Currently just prints a tick message.
 */
static void timer_interrupt_handler(struct interrupt_state *state) {
    (void) state;   /** Unused. */

    ticks++;
}


/**
 * Initialize the PIT timer. Registers timer interrupt ISR handler, sets
 * PIT to run in mode 3 with given frequency in Hz.
 */
void timer_init(void) {
    isr_register(IDT_IRQ_BASE + IDT_IRQ_TIMER, &timer_interrupt_handler);

    /**
     * Calculate the frequency divisor needed to run with the given
     * frequency. Divisor = base frequencty / desired frequency.
     */
    uint16_t divisor = TIMER_FREQ_BASE_HZ / TIMER_FREQ_HZ;

    // 00110100b = 00 channel 0, 11 lobyte/hibyte, 010 mode 2, 0 binary.
    outb(IO_PORT_TIMER_CMD, 0x34);

    /** Sends frequency divisor, in lo | hi order. */
    outb(IO_PORT_TIMER_DATA0, (uint8_t) (divisor & 0xFF));
    outb(IO_PORT_TIMER_DATA0, (uint8_t) ((divisor >> 8) & 0xFF));

    pic_enable_irq_line(PIC_INT_TIMER);
}
