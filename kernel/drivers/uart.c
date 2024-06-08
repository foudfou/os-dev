// Intel 8250 serial port (UART).

#include "drivers/ioapic.h"
#include "drivers/screen.h"
#include "idt.h"
#include "low_level.h"

#include "drivers/uart.h"

#define COM1    0x3f8

static int uart;    // is there a uart?

// xv6 lapic.c
// Spin for a given number of microseconds.
// On real hardware would want to tune this dynamically.
void microdelay(int us) { }

void
uartputc(uint8_t c)
{
  if(!uart)
    return;
  for(int i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++)
    microdelay(10);
  outb(COM1+0, c);
}

static uint8_t
uartgetc(void)
{
  if(!uart)
    return -1;
  if(!(inb(COM1+5) & 0x01))
    return -1;
  return inb(COM1+0);
}

static void com1_interrupt_handler(struct interrupt_state *state) {
    (void) state;   /** Unused. */

    // FIXME merge with kbd_interrupt_handler(). See xv6 consoleintr().
    uartputc(uartgetc());
}

void uartinit(void) {
  isr_register(IDT_IRQ_BASE + IDT_IRQ_COM1, &com1_interrupt_handler);

  // Turn off the FIFO
  outb(COM1+2, 0);

  // 9600 baud, 8 data bits, 1 stop bit, parity off.
  outb(COM1+3, 0x80);    // Unlock divisor
  outb(COM1+0, 115200/9600); // Set divisor (lo byte)
  outb(COM1+1, 0);           // Set divisor (hi byte)
  outb(COM1+3, 0x03);    // Lock divisor, 8 data bits.
  outb(COM1+4, 0);
  outb(COM1+1, 0x01);    // Enable receive interrupts.

  // If status is 0xFF, no serial port.
  if(inb(COM1+5) == 0xFF)
    return;
  uart = 1;

  // Acknowledge pre-existing interrupt conditions;
  // enable interrupts.
  inb(COM1+2);
  inb(COM1+0);
  ioapicenable(IDT_IRQ_COM1, 0);

  // Announce that we're here.
  for(char *p = "xv6...\n"; *p; p++)
    uartputc(*p);
}
