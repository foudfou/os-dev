#ifndef IOAPIC_H
#define IOAPIC_H


#include <stdint.h>

void ioapicinit(void);

void ioapicenable(uint32_t irq, uint32_t cpunum);


#endif /* IOAPIC_H */
