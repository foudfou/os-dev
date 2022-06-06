#include "kernel/idt.h"
#include "kernel/low_level.h"
#include "kernel/pic.h"

/** Send back PIC end-of-interrupt (EOI) signal. */
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(IO_PORT_PIC2_COMMAND, PIC_EOI);
    outb(IO_PORT_PIC1_COMMAND, PIC_EOI);
}

/* Note when a bit is set, the PIC ignores the request and continues normal operation.  */
void pic_enable_irq_line(int numirq) {
    uint16_t port;
    uint8_t mask;

    if(numirq < 8) {
        port = IO_PORT_PIC1_DATA;
    } else {
        port = IO_PORT_PIC2_DATA;
        numirq -= 8;
    }
    mask = inb(port);
    mask = mask & ~(1 << numirq);
    outb(port, mask);
}

void pic_disable_irq_line(int numirq) {
    uint16_t port;
    uint8_t mask;

    if(numirq < 8) {
        port = IO_PORT_PIC1_DATA;
    } else {
        port = IO_PORT_PIC2_DATA;
        numirq -= 8;
    }
    mask = inb(port) | (1 << numirq);
    outb(port, mask);
}

void pic_init() {
    // save masks
    uint8_t mask1 = inb(IO_PORT_PIC1_DATA);
    uint8_t mask2 = inb(IO_PORT_PIC2_DATA);
    // not sure why mask1=B8,mask2=8E/8F at startup in bochs/qemu

    /* Send ICW1: 8086 mode + NOT Single ctrl + call address interval=8 */
    outb(IO_PORT_PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); // starts the initialization sequence (in cascade mode)
    outb(IO_PORT_PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    /* Send ICW2: ctrl base address */
    outb(IO_PORT_PIC1_DATA, PIC1_INTERRUPT_START); // ICW2: Master PIC vector offset
    outb(IO_PORT_PIC2_DATA, PIC2_INTERRUPT_START); // ICW2: Slave PIC vector offset

    /* Send ICW3 master: mask where slaves are connected */
    outb(IO_PORT_PIC1_DATA, 4); // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outb(IO_PORT_PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)

    /* Send ICW4: 8086 mode, fully nested, not buffered, no implicit EOI */
    outb(IO_PORT_PIC1_DATA, ICW4_8086);
    outb(IO_PORT_PIC2_DATA, ICW4_8086);

    outb(IO_PORT_PIC1_DATA, mask1);   // restore saved masks.
    outb(IO_PORT_PIC2_DATA, mask2);

    pic_disable_irq_line(PIC_INT_TIMER);
}
