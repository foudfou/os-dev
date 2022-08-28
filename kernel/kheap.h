/**
 * Kernel Heap Memory "Next-Fit" Allocator.
 */
#ifndef KHEAP_H
#define KHEAP_H

#include <stddef.h>
#include <stdint.h>

/**
 * The free list is embedded inside the heap. Every allocated object (i.e.,
 * memory chunk returned to caller of `kalloc()`) is prefixed with a free-list
 * header structure.
 *
 * Not using https://github.com/foudfou/ptp/blob/master/src/utils/list.h as we
 * don't need a doubly-linked list.
 */
struct kheap_node {
    size_t size;
    struct kheap_node *next;
};

#define KHEAP_NODE_SIZE sizeof(struct kheap_node)

/** Random magic number to protect against memory overruns. */
#define KHEAP_MAGIC (struct kheap_node *)0xFFCABCAB

/** Pointer arithmetics helper macros. */
#define HEADER_TO_OBJECT(header) (void *)((header) + KHEAP_NODE_SIZE)
#define OBJECT_TO_HEADER(object) (struct kheap_node *)((object) - KHEAP_NODE_SIZE)


void kheap_init();

uint32_t kalloc(size_t size);
void kfree(void *addr);


#endif
