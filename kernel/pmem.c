#include "drivers/screen.h"
#include "lib/bitmap.h"
#include "lib/debug.h"
#include "lib/string.h"

#include "kernel/pmem.h"

/* Well-known memory regions */
#define PMEM_EXTENDED_ADDR 0x100000 // aka "high mem"

extern char __k_start, __k_end;

/** Bitmap tracking used frames. */
static bitmap *frame_map = NULL;
static uint64_t num_frames = 0;

/** Kernel heap bottom address - starts above kernel code. */
static uintptr_t kheap_curr;

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

/**
 * Auxiliary function for allocating (page-aligned) chunks of memory in the
 * kernel heap region that never gets freed.
 *
 * Should only be used to allocate the kernel's page directory/tables and
 * the frames bitmap and other things before our actual heap allocation
 * algorithm setup.
 */
uintptr_t
kalloc_temp(const size_t size, bool page_align)
{
    /** If `page_align` is set, return an aligned address. */
    if (page_align && !ADDR_PAGE_ALIGNED(kheap_curr))
        kheap_curr = ADDR_PAGE_ROUND_UP(kheap_curr);

    /** If exceeds the 8MiB kernel memory boundary, panic. */
    if (kheap_curr + size > KHEAP_MAX_ADDR)
        error("_kalloc_temp: kernel memory exceeds boundary");

    uintptr_t temp = kheap_curr;
    kheap_curr += size;
    return temp;
}

uint64_t frame_alloc() {
    return bitmap_alloc(frame_map, num_frames);
}

void pmem_init(const struct pmem_info *info) {
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
        panic("external memory not found");

    /* Kernel heap starts after kernel code. */
    kheap_curr = ADDR_PAGE_ROUND_UP((uintptr_t) &__k_end);
    cprintf("kernel heap starting at 0x%p\n", kheap_curr);

    num_frames = extmem->len / PAGE_SIZE; // extmem->len >> PAGE_SIZE_SHIFT
    size_t frame_map_size_in_bytes = num_frames / 8;
    frame_map = (bitmap *)kalloc_temp(frame_map_size_in_bytes, false);
    memset(frame_map, 0, frame_map_size_in_bytes);
}
