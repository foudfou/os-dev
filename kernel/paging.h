#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "kernel/pmem.h"


// Control Register flags
#define CR0_PE          0x00000001      // Protection Enable
#define CR0_WP          0x00010000      // Write Protect
#define CR0_PG          0x80000000      // Paging


// A virtual address 'va' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/

#define PDXSHIFT        22      // offset of PDX in a linear address
#define PTXSHIFT        12      // offset of PTX in a linear address

// page directory index
#define PDX(va)         (((va) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(va)         (((va) >> PTXSHIFT) & 0x3FF)

// construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((uint32_t)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

// Page directory and page table constants.
#define NPDENTRIES      1024    // # directory entries per page directory
#define NPTENTRIES      1024    // # PTEs per page table
#define PGSIZE          4096    // bytes mapped by a page

#define PGROUNDUP(sz)  (((uint32_t)(sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((uint32_t)(a)) & ~(PGSIZE-1))

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PS          0x080   // Page Size

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uint32_t)(pte) & ~0xFFF)
#define PTE_FLAGS(pte)  ((uint32_t)(pte) &  0xFFF)


// Key addresses for address space layout (see kmap in vm.c for layout)
#define KERNBASE 0x80000000         // First kernel virtual address
/* #define KERNBASE 0 */
#define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked

#define V2P(a) (((uint32_t) (a)) - KERNBASE)
#define P2V(a) ((void *)(((char *) (a)) + KERNBASE))

/**
 * Helper macro to control the MMU: invalidate the TLB entry for the
 * page located at the given virtual address. See Intel x86 vol 3
 * section 3.7.
 */
#define invlpg(vaddr)                                                   \
    do {                                                                \
        __asm__ __volatile__("invlpg %0"::"m"(*((unsigned *)(vaddr)))); \
    } while(0)


/**
 * Helper macro to control the MMU: invalidate the whole TLB. See
 * Intel x86 vol 3 section 3.7.
 */
#define flush_tlb()                                                 \
    do {                                                            \
        unsigned long tmpreg;                                       \
        __asm__ __volatile__("movl %%cr3,%0\n\tmovl %0,%%cr3" :"=r" \
                             (tmpreg) : :"memory");                 \
    } while (0)



/**
 * Page directory and table entry really are 32-bit page-aligned addresses. We
 * could defined packed structs (field definition order is LSB -> MSB), as in
 * https://github.com/josehu07/hux-kernel/blob/main/src/memory/paging.h#L48,
 * but it's just easier to use uint32_t directly.
 */
typedef uint32_t pde_t;
typedef uint32_t pte_t;

/** Extern the kernel page directory pointer to the scheduler. */
extern pde_t *kpgdir;

uint32_t paging_get_paddr(uint32_t vaddr);

void paging_switch_pgdir(const pde_t *pgdir);

void paging_init();

#endif /* PAGING_H */
