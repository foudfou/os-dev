#include "drivers/screen.h"
#include "lib/string.h"

#include "kernel/pmem.h"

/* Well-known memory regions */
#define PMEM_EXTENDED_ADDR 0x100000 // aka "high mem"

/** Bitmap tracking used frames. */
static uint32_t *frame_map = NULL;

uint64_t num_frames = 0;

/* Checks if A20 is enabled
 *
 * https://forum.osdev.org/viewtopic.php?p=276550#p276550
 *
 * As of 2022-07, it seems impossible to disable A20 for testing in bochs and
 * qemu. Fortunately virtualbox doesn't enable A20.
 * https://github.com/margaretbloom/disable-a20.
 */
static bool a20_enabled() {
    uint32_t *r = (uint32_t*)0x012345;
    uint32_t *w = (uint32_t*)0x112345;

    *r = 0x87654321;
    *w = 0x12345678;
    return *r != *w;
}

void pmem_init(const struct pmem_info *info, uint32_t *phys_end) {
    if (!a20_enabled())
        panic("A20 line not enabled");

    if (!info || !info->cnt)
        panic("physical memory map not initialized");

    const struct e820_entry *extmem = NULL;
    print("BIOS-provided physical RAM map:\n");
    for (int i = 0; i < info->cnt; i++) {
        const struct e820_entry *entry = &info->entries[i];
        cprintf("BIOS-e820: [base=%l length=%l] type=%d\n",
                entry->base, entry->len, entry->type);

        if (!extmem &&
            entry->type == E820_TYPE_RAM &&
            entry->base >= PMEM_EXTENDED_ADDR) {
            extmem = entry;
        }
    }

    if (!extmem)
        panic("extended memory not found");

    *phys_end = (uint32_t)(extmem->base + extmem->len);
}
