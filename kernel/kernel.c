#include "drivers/kbd.h"
#include "drivers/screen.h"
#include "drivers/timer.h"
#include "kernel/idt.h"
#include "kernel/kheap.h"
#include "kernel/paging.h"
#include "kernel/pic.h"
#include "kernel/pmem.h"

#include "lib/string.h"

void main(const struct pmem_info *pmem_info) {
    clear_screen();
    print("FOUDIL WAS HERE\n(c) 2022\n");

    uint64_t ram_size = 0;
    pmem_init(pmem_info, &ram_size);
    cprintf("kernel heap starting at 0x%p\n", kheap_curr);
    cprintf("Available memory: %l bytes\n", ram_size);

    idt_init();
    print("IDT initialized\n");

    pic_init();
    print("PIC initialized\n");

    timer_init();      // PIT timer support
    print("PIT Timer initialized\n");

    kbd_init();        // PS/2 keyboard support
    print("PS/2 keyboard initialized\n");

    paging_init(ram_size);
    print("Pagination enabled\n");

    kheap_init();
    print("kernel heap memory allocator initialized\n");

    __asm__("sti");    // Enable interrupts

    // TESTING kalloc

    cprintf("kalloc arr1 - 128 bytes...\n");
    char *arr1 = (char *) kalloc(128 * sizeof(char));
    strncpy(arr1, "hello\n", 127);

    cprintf("kalloc arr2 - 23 bytes...\n");
    char *arr2 = (char *) kalloc(23 * sizeof(char));
    strncpy(arr2, "hello\n", 22);

    cprintf("kalloc arr3 - 437 bytes...\n");
    char *arr3 = (char *) kalloc(437 * sizeof(char));
    strncpy(arr3, "hello\n", 436);

    cprintf("kfree arr3, should coalesce with the big chunk...\n");
    kfree(arr3);

    cprintf("kfree arr1, should have no coalescing...\n");
    kfree(arr1);

    cprintf("kalloc arr4 - 54 bytes, should reuse the first chunk...\n");
    char *arr4 = (char *) kalloc(54 * sizeof(char));
    strncpy(arr4, "hello\n", 53);

    cprintf("kfree arr2, should coalesce with both neighbors...\n");
    kfree(arr2);

    struct kheap_node *hdr = OBJECT_TO_HEADER(arr4);
    cprintf("  BEFORE: arr4=%p, hdr=%p {.szie=%x, .next=%x}\n", arr4, hdr, hdr->size, hdr->next);
    strncpy((char *)&hdr->next, "AAAA", sizeof(uint32_t));
    cprintf("  AFTER:  arr4=%p, hdr=%p {.szie=%x, .next=%x}\n", arr4, hdr, hdr->size, hdr->next);

    cprintf("kfree arr4, should trigger magic failure...\n");
    kfree(arr4);

    while (1)   // CPU idles
        __asm__ __volatile__( "hlt" );
}
