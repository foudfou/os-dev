#ifndef PMEM_H
#define PMEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Upper bound of kernel heap.

    The kernel heap will start after the kernel code. Since the kernel is
    loaded to extended memory, we should have plenty of space. We could also
    determine this from the e820 memory map. */
#define KHEAP_MAX_ADDR 0x800000   // 8MiB reserved for the kernel

/*
  – x << PAGE_SIZE_SHIFT <=> x * PAGE_SIZE
  – x >> PAGE_SIZE_SHIFT <=> x / PAGE_SIZE
  – x & PAGE_SIZE_MASK <=> x % PAGE_SIZE
*/
#define PAGE_SIZE_SHIFT 12
#define PAGE_SIZE       (1<<PAGE_SIZE_SHIFT)
#define PAGE_MASK       ((1<<PAGE_SIZE_SHIFT) - 1)

/** Helper macros on addresses and page alignments. */
#define ADDR_PAGE_OFFSET(addr) ((addr) & 0x0FFF)
#define ADDR_PAGE_NUMBER(addr) ((addr) >> PAGE_SIZE_SHIFT)

#define ADDR_PAGE_ALIGNED(addr) (ADDR_PAGE_OFFSET(addr) == 0)

#define ADDR_PAGE_ROUND_DN(addr) ((addr) & 0xFFFFF000)
#define ADDR_PAGE_ROUND_UP(addr) (ADDR_PAGE_ROUND_DN((addr) + 0x00000FFF))

struct e820_entry {
    uint64_t base;
    uint64_t len;
    uint32_t type;
    uint32_t acpi;
} __attribute__((packed));

enum e820_type {
    E820_TYPE_RAM       = 1,
    E820_TYPE_RESERVED  = 2,
    E820_TYPE_ACPI      = 3,
    E820_TYPE_NVS       = 4,
    E820_TYPE_UNUSABLE  = 5,
    E820_TYPE_PMEM      = 7,
};

struct pmem_info {
    uint32_t          cnt;
    struct e820_entry entries[];
};

uintptr_t kalloc_temp(const size_t size, bool page_align);
uint64_t frame_alloc();
void pmem_init(const struct pmem_info *info);


#endif /* PMEM_H */
