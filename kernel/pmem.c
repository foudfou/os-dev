#include "drivers/screen.h"
#include "kernel/pmem.h"
#include "lib/debug.h"

void pmem_init(const struct pmem_info *info) {
    if (!info || !info->cnt)
        panic("physical memory map not initialized");

    print("BIOS-provided physical RAM map:\n");
    for (int i = 0; i < info->cnt; i++) {
        const struct e820_entry *entry = &info->entries[i];
        cprintf("BIOS-e820: [base=%l length=%l] type=%d\n",
                entry->base, entry->len, entry->type);
    }
}
