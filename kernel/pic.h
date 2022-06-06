#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define IO_PORT_PIC1_COMMAND 0x20
#define IO_PORT_PIC1_DATA    0x21
#define IO_PORT_PIC2_COMMAND 0xA0
#define IO_PORT_PIC2_DATA    0xA1

#define PIC_EOI 0x20            /* End Of Interruption */

#define ICW1_ICW4      0x01     /* ICW4 (not) needed */
#define ICW1_SINGLE    0x02     /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04     /* Call address interval 4 (8) */
#define ICW1_LEVEL     0x08     /* Level triggered (edge) mode */
#define ICW1_INIT      0x10     /* Initialization - required! */

#define ICW4_8086       0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02    /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C    /* Buffered mode/master */
#define ICW4_SFNM       0x10    /* Special fully nested (not) */

#define PIC1_INTERRUPT_START 0x20
#define PIC2_INTERRUPT_START 0x28

#define PIC_INT_TIMER    0
#define PIC_INT_KEYBOARD 1

void pic_send_eoi(uint8_t irq);
void pic_init();
void pic_enable_irq_line(int numirq);
void pic_disable_irq_line(int numirq);


#endif /* PIC_H */
