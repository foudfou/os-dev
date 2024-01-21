#ifndef GDT_H
#define GDT_H

#include <stdint.h>

// Segment selectors. Null segment descriptor (index 0) is not used by the CPU.
#define SEG_KCODE 1  // kernel code
#define SEG_KDATA 2  // kernel data+stack
#define SEG_UCODE 3  // user code
#define SEG_UDATA 4  // user data+stack
#define SEG_TSS   5  // this process's task state

#define SEG_PRESENT      0b10000000
#define SEG_RING0        0b00000000
#define SEG_RING3        0b01100000
/** Application segments
 * 7   6   5   4   3   2   1   0
 * P   DPL     S   E   DC  RW  A
 */
#define SEG_APP          0b00010000 // otherwise system
#define SEG_CODE         0b00001000 // otherwise data
#define SEG_DIR_CFM      0b00000100
#define SEG_RW           0b00000010 // code-read or data-write
#define SEG_ACCESS       0b00000001
/** Application segments
 * 7   6   5   4   3   2   1   0
 * P   DPL     S   Type
 */
#define SEG_TSS32_AVAIL  0b00001001

#define DPL_USER 3 // (SEG_RING3 >> 5)

#define SEG_32BIT        0b01000000
#define SEG_4K           0b10000000

#define NSEGS     6

struct segdesc {
    uint16_t limit_lo;
    uint16_t base_lo;
    uint8_t base_mi;
    uint8_t access;
    uint8_t flags;         // lim_hi(4 lower bits)
    uint8_t base_hi;
} __attribute__((packed)); // `packed` is not strictly necessary since the
                           // struct is already 32-bit aligned.

struct gdtr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)); // here `packed` is critical!

void gdt_set_entry(struct segdesc *seg, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdt_init(void);

#endif /* GDT_H */
