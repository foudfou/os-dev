#ifndef LOW_LEVEL_H
#define LOW_LEVEL_H

#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    // A handy C wrapper function that reads a byte from the specified port
    // "=a" (result) means: put AL register in variable RESULT when finished
    // "d" (port) means: load EDX with port
    uint8_t result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

static inline void outb(uint16_t port, uint8_t data) {
    // "a" (data) means: load EAX with data
    // "d" (port) means: load EDX with port
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

static inline void outw(uint16_t port, uint16_t data) {
    __asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}

static inline void insl(uint16_t port, void *addr, uint32_t cnt) {
  __asm__ __volatile__("cld; rep insl" :
                       "=D" (addr), "=c" (cnt) :
                       "d" (port), "0" (addr), "1" (cnt) :
                       "memory", "cc");
}

/** Output CNT 32-bit dwords from the buffer at ADDR to an I/O port. */
static inline void outsl(uint16_t port, const void *addr, uint32_t cnt) {
    __asm__ __volatile__ ( "cld; rep outsl"
                           : "+S" (addr), "+c" (cnt)
                           : "d" (port) );
}

#endif /* LOW_LEVEL_H */
