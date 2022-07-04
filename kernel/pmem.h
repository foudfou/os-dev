#ifndef PMEM_H
#define PMEM_H

#include <stdint.h>

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

void pmem_init(const struct pmem_info *info);


#endif /* PMEM_H */
