/**
 * Kernel Heap Memory "Next-Fit" Allocator.
 */


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "lib/debug.h"
#include "lib/string.h"
#include "paging.h"
#include "pmem.h"

#include "kheap.h"


static uint32_t kheap_btm;
static uint32_t kheap_top;


/** Global state of the free-list. */
static struct kheap_node *kheap_nodes;
static struct kheap_node *last_search_node; /** Where the last search ends. */
// FIXME is len really needed?
static size_t kheap_nodes_len = 1;



/** For debug printing the state of the free-list. */
__attribute__((unused))
static void
_print_kheap_state(void)
{
    /* spinlock_acquire(&kheap_lock); */

    struct kheap_node *hdr = kheap_nodes;

    info("kheap free-list length = %d, last_search = %p, nodes:",\
         kheap_nodes_len, last_search_node);
    do {
        cprintf("  node header %p { size: %d, next: %p }\n",
                hdr, hdr->size, hdr->next);

        hdr = hdr->next;
    } while (hdr != kheap_nodes);

    /* spinlock_release(&kheap_lock); */
}


/**
 * Allocate a piece of heap memory (an object) of given size. Adopts
 * the "next-fit" allocation policy. Returns 0 on failure.
 */
uint32_t kalloc(size_t size) {
    /* spinlock_acquire(&kheap_lock); */

    if (kheap_nodes_len == 0) {
        warn("kalloc: kernel flexible heap all used up");
        /* spinlock_release(&kheap_lock); */
        return 0;
    }

    struct kheap_node *last = last_search_node;
    struct kheap_node *curr = (struct kheap_node *) last->next;
    struct kheap_node *begin = curr;

    do {
        /** If this node is not large enough, skip. */
        if (curr->size < size) {
            last = curr;
            curr = (struct kheap_node *) curr->next;
            continue;
        }

        /**
         * Else, split this node if it is larger than `size` + meta bytes
         * (after split, the rest of it can be made a smaller free chunk).
         */
        if (curr->size > size + KHEAP_NODE_SIZE) {
            /** Split out a new node. */
            struct kheap_node *new = (struct kheap_node *)
                ((uint32_t) curr + size + KHEAP_NODE_SIZE);
            new->size = curr->size - size - KHEAP_NODE_SIZE;
            new->next = KHEAP_MAGIC;

            /** Don't forget to update the current node's size. */
            curr->size = size;

            /**
             * Now, update the links between nodes. The special case of list
             * only having one node needs to be taken care of.
             *
             * If only one node in list, then `last` == `curr`,
             * so `last_search_node` should be the new node, not the current
             * node (that is about to be returned as an object).
             */
            if (kheap_nodes_len == 1) {
                new->next = new;
                last_search_node = new;
            } else {
                new->next = curr->next;
                last->next = new;
                last_search_node = last;
            }

            /** Update smallest-address node. */
            if (curr == kheap_nodes)
                kheap_nodes = new;

        }
        // size <= curr->size <= size + KHEAP_NODE_SIZE
        else {
            /** Not splitting, then just take this node off the list. */
            last->next = curr->next;
            kheap_nodes_len--;

            /** Update smallest-address node. */
            if (curr == kheap_nodes)
                kheap_nodes = curr->next;
        }

        /** `curr` is now the chunk to return. */
        curr->next = KHEAP_MAGIC;
        uint32_t object = HEADER_TO_OBJECT((uint32_t) curr);

        /* spinlock_release(&kheap_lock); */

        _print_kheap_state();
        return object;

    } while (curr != begin);

    /** No free chunk is large enough, time to panic. */
    warn("kalloc: no free chunk large enough for size %d\n", size);
    /* spinlock_release(&kheap_lock); */
    return 0;
}


/**
 * Free a previously allocated object address, and merges it into the
 * free-list if any neighboring chunk is free at the time.
 */
void kfree(void *addr) {
    struct kheap_node *hdr = (struct kheap_node *) OBJECT_TO_HEADER((uint32_t) addr);

    if ((uint32_t) addr < kheap_btm || (uint32_t) addr >= kheap_top) {
        error("kfree: object %p is out of heap range", addr);
        return;
    }

    if (hdr->next != KHEAP_MAGIC) {
        error("kfree: object %p corrupted its header magic", addr);
        return;
    }

    /** Fill with zero bytes to catch dangling pointers use. */
    memset((char *) addr, 0, hdr->size);

    /* spinlock_acquire(&kheap_lock); */

    /**
     * Special case of empty free-list (all bytes exactly allocated before
     * this `kfree()` call). If so, just add this free'd obejct as a node.
     */
    if (kheap_nodes_len == 0) {
        hdr->next = hdr;
        kheap_nodes = hdr;
        last_search_node = hdr;
        kheap_nodes_len++;
    }
    /**
     * Else, traverse the free-list starting form the bottom-most node to
     * find the proper place to insert this free'd node, and then check if
     * any of its up- & down-neighbor is free. If so, coalesce with them
     * into a bigger free chunk.
     *
     * This linked-list traversal is not quite efficient and there are tons
     * of ways to optimize the free-list structure, e.g., using a balanced
     * binary search tree. Left as future work.
     */
    else {
        /**
         * Locate down-neighbor. Will get the top-most node if the free'd
         * object is located below the current bottom-most node.
         */
        struct kheap_node *dn_hdr = kheap_nodes;
        while (dn_hdr->next != kheap_nodes) {
            if (dn_hdr < hdr && dn_hdr->next > hdr)
                break;
            dn_hdr = dn_hdr->next;
        }
        bool dn_coalesable = dn_hdr > hdr ? false
            : HEADER_TO_OBJECT((uint32_t) dn_hdr) + dn_hdr->size == (uint32_t) hdr;

        /**
         * Locate up-neighbor. Will get the bottom-most node if the free'd
         * object is located above all the free nodes.
         */
        struct kheap_node *up_hdr = dn_hdr > hdr ? kheap_nodes
            : dn_hdr->next;
        bool up_coalesable = up_hdr < hdr ? false
            : HEADER_TO_OBJECT((uint32_t) hdr) + hdr->size == (uint32_t) up_hdr;

        /** Case #1: Both coalesable. */
        if (dn_coalesable && up_coalesable) {
            /** Remove up-neighbor from list. */
            dn_hdr->next = up_hdr->next;
            kheap_nodes_len--;
            /** Extend down-neighbor to cover the whole region. */
            dn_hdr->size +=
                hdr->size + up_hdr->size + 2 * KHEAP_NODE_SIZE;
            /** Update last search node. */
            if (last_search_node == up_hdr)
                last_search_node = dn_hdr;
        }

        /** Case #2: Only with down-neighbor. */
        else if (dn_coalesable) {
            /** Extend down-neighbor to cover me. */
            dn_hdr->size += hdr->size + KHEAP_NODE_SIZE;
        }

        /** Case #3: Only with up neighbor. */
        else if (up_coalesable) {
            /** Update dn-neighbor to point to me. */
            dn_hdr->next = hdr;
            /** Extend myself to cover up-neighbor. */
            hdr->size += up_hdr->size + KHEAP_NODE_SIZE;
            hdr->next = up_hdr->next;
            /** Update bottom-most node. */
            if (kheap_nodes > hdr)
                kheap_nodes = hdr;
            /** Update last search node. */
            if (last_search_node == up_hdr)
                last_search_node = hdr;
        }

        /** Case #4: With neither. */
        else {
            /** Link myself in the list. */
            dn_hdr->next = hdr;
            hdr->next = up_hdr;
            kheap_nodes_len++;
            /** Update bottom-most node. */
            if (kheap_nodes > hdr)
                kheap_nodes = hdr;
        }
    }

    /* spinlock_release(&kheap_lock); */
    _print_kheap_state();
}


/** Initialize the kernel heap allocator. */
void
kheap_init(void)
{
    /**
     * Initially, everything from `kheap_curr` (page rounded-up) upto 8MiB
     * are free heap memory available to use. Initialize it as one big
     * free chunk.
     */
    kheap_btm = kheap_curr;
    kheap_top = KHEAP_MAX_ADDR;

    struct kheap_node *header = (struct kheap_node *) kheap_btm;
    uint32_t size = (kheap_top - kheap_btm) - KHEAP_NODE_SIZE;
    memset((char *) (HEADER_TO_OBJECT(kheap_btm)), 0, size);

    header->size = size;
    header->next = header;      /** Point to self initially. */

    kheap_nodes = header;
    last_search_node = header;
    kheap_nodes_len = 1;

    /* spinlock_init(&kheap_lock, "kheap_lock"); */
}
