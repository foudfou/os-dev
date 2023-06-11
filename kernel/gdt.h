#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// various segment selectors.
#define SEG_KCODE 1  // kernel code
#define SEG_KDATA 2  // kernel data+stack
#define SEG_UCODE 3  // user code
#define SEG_UDATA 4  // user data+stack
#define SEG_TSS   5  // this process's task state

#define SEG_PRESENT      0b00010000
#define SEG_CODE_OR_DATA 0b10000000
#define SEG_RING0        0b00000000
#define SEG_RING3        0b01100000
#define SEG_CODE         0b00001000
#define SEG_READ         0b00000010

// Convenient combination
#define SEG_PREAD SEG_READ|SEG_CODE_OR_DATA|SEG_PRESENT

#define SEG_32BIT        0b01000000
#define SEG_4K           0b10000000

#define NSEGS     6

struct segdesc {
    uint16_t limit_lo;
    uint16_t base_lo;
    uint8_t base_mi;
    uint8_t access;
    uint8_t flags;
    uint8_t base_hi;
} __attribute__((packed)); // `packed` is not strictly necessary since the
                           // struct is already 32-bit aligned.

struct gdtr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)); // here `packed` is critical!

void gdt_init(void);

#endif /* GDT_H */
