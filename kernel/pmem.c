#include "drivers/screen.h"
#include "lib/bitmap.h"
#include "lib/debug.h"
#include "lib/string.h"

#include "kernel/pmem.h"

/* Well-known memory regions */
#define PMEM_EXTENDED_ADDR 0x100000 // aka "high mem"

extern char __k_start, __k_end;

/** Bitmap tracking used frames. */
static uint32_t *frame_map = NULL;

uint64_t num_frames = 0;

/** Kernel heap bottom address - starts above kernel code. */
uint32_t kheap_curr;

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
        error("kalloc_temp: kernel memory exceeds boundary");

    uintptr_t temp = kheap_curr;
    kheap_curr += size;
    return temp;
}

uint64_t frame_alloc() {
    return bitmap_alloc(frame_map, num_frames);
}

void pmem_init(const struct pmem_info *info, uint64_t *ram_size) {
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

    *ram_size = extmem->len;

    /* Allocate space for the frame bitmap in our kernel heap. */
    num_frames = *ram_size / PAGE_SIZE; // *ram_size >> PAGE_SIZE_SHIFT
    // frame_map is a bitmap, i.e. an uint32_t array. An uint32_t, 4 bytes, can
    // store 32 frames. How many bytes do we need to store num_frames?
    size_t frame_map_size_in_u32 = num_frames / 32;
    // Fortunately the compliler doesn't optimize to `num_frames / 8` which is
    // not the same!
    size_t frame_map_size_in_bytes = frame_map_size_in_u32 * 4;
    frame_map = (uint32_t*)kalloc_temp(frame_map_size_in_bytes, false);
    memset(frame_map, 0, frame_map_size_in_bytes);
}
