// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.
/* Lifted from https://github.com/mit-pdos/xv6-public/blob/master/kalloc.c */

#include "drivers/screen.h"
#include "lib/string.h"
#include "kernel/paging.h"
#include "kernel/spinlock.h"

#include "kernel/kalloc.h"

#define KMEM_SENTINEL (struct frame *)&kmem.freelist


/** Kernel heap address range. Starts above kernel code. */
uint32_t kheap_start;
uint32_t kheap_end = 0x07fe0000; // overriden dynamically

struct frame {
  struct frame *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct frame *freelist;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  kheap_start = (uint32_t)vstart;

  // Be explicit about the sentinel value. Xv6 uses 0.
  kmem.freelist = KMEM_SENTINEL;

  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;

  freerange(vstart, vend);
  // dump_freelist();
}

void
kinit2(void *vstart, void *vend)
{
  kheap_end = (uint32_t)vend;
  freerange(vstart, vend);
  kmem.use_lock = 1;
  dump_freelist();
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint32_t)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
//
// Note that the free list starts with the highest numbered free frame and
// begins sorted in reverse order.
void
kfree(char *v)
{
  struct frame *r;

  if((uint32_t)v % PGSIZE || (uint32_t)v < kheap_start || V2P(v) >= kheap_end)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct frame*)v;
  r->next = kmem.freelist;      // points to the previous
  kmem.freelist = r;            // points to the last
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct frame *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if (r == KMEM_SENTINEL)
    r = 0;
  else
    kmem.freelist = r->next;
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

void dump_freelist() {
    struct frame *p = kmem.freelist;
    int nframes = 0;
    cprintf("kernel heap start: 0x%p\n", p);
    while (p->next != KMEM_SENTINEL) {
        p = p->next;
        nframes++;
    }
    cprintf("kernel heap end: 0x%p\n", p);
    cprintf("number of free frames: %d\n", nframes);
}
