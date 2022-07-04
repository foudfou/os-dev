#include <stdbool.h>
#include "drivers/screen.h"
#include "kernel/pmem.h"
#include "lib/bitmap.h"
#include "lib/debug.h"
#include "lib/string.h"

/* Well-known memory regions */
#define PMEM_EXTENDED_ADDR 0x100000 // aka "high mem"

/** Upper bound of kernel heap. */
// #define KHEAP_MAX_ADDR 0x00800000   // 8MiB reserved for the kernel
/* Our kernel is loaded in low memory and the kernel heap will start after the
   kernel code. We thus need to be frugal. We could also determine this from
   the e820 memory map. */
#define KHEAP_MAX_ADDR 0x7FFFF // end of conventional memory, roughly 256 KiB

/*
  – x << PAGE_SIZE_SHIFT <=> x * PAGE_SIZE
  – x >> PAGE_SIZE_SHIFT <=> x / PAGE_SIZE
  – x & PAGE_SIZE_MASK <=> x % PAGE_SIZE
*/
#define PAGE_SIZE_SHIFT 12
#define PAGE_SIZE       (1<<PAGE_SIZE_SHIFT)
#define PAGE_MASK       ((1<<PAGE_SIZE_SHIFT) - 1)

#define PAGE_ALIGNED __attribute__((aligned(PAGE_SIZE)))

/** Helper macros on addresses and page alignments. */
#define ADDR_PAGE_OFFSET(addr) ((addr) & 0x0FFF)
#define ADDR_PAGE_NUMBER(addr) ((addr) >> PAGE_SIZE_SHIFT)

#define ADDR_PDE_INDEX(addr) (ADDR_PAGE_NUMBER(addr) / 1024)
#define ADDR_PTE_INDEX(addr) (ADDR_PAGE_NUMBER(addr) % 1024)

#define ADDR_PAGE_ALIGNED(addr) (ADDR_PAGE_OFFSET(addr) == 0)

#define ADDR_PAGE_ROUND_DN(addr) ((addr) & 0xFFFFF000)
#define ADDR_PAGE_ROUND_UP(addr) (ADDR_PAGE_ROUND_DN((addr) + 0x00000FFF))

extern char __k_start, __k_end;

/** Bitmap tracking used frames. */
static bitmap *frame_map = NULL;
static uint64_t num_frames = 0;

/** Kernel heap bottom address - starts above kernel code. */
static uintptr_t kheap_curr;

/**
 * Auxiliary function for allocating (page-aligned) chunks of memory in the
 * kernel heap region that never gets freed.
 *
 * Should only be used to allocate the kernel's page directory/tables and
 * the frames bitmap and other things before our actual heap allocation
 * algorithm setup.
 */
static uintptr_t
_kalloc_temp(const size_t size, bool page_align)
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
    frame_map = (bitmap *)_kalloc_temp(frame_map_size_in_bytes, false);
    memset(frame_map, 0, frame_map_size_in_bytes);

    // FIXME testing
    uint64_t frame_idx = num_frames;
    for (int i = 0; i < 1000; ++i)
        frame_idx = bitmap_alloc(frame_map, num_frames);
    cprintf("frame_idx=%l\n", frame_idx);
}
