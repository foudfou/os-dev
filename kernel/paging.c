#include "drivers/screen.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "kernel/idt.h"
#include "kernel/kalloc.h"
#include "kernel/pmem.h"

#include "kernel/paging.h"

/** kernel's identity-mapping page directory. */
pde_t *kpgdir;    /** Allocated at paging init. */


// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va. If alloc!=0,
// create any required page table pages.
/* Lifted from https://github.com/mit-pdos/xv6-public/blob/master/vm.c */
static pte_t *
walkpgdir(pde_t *pgdir, const uint32_t va, bool alloc)
{
    pde_t *pde;
    pte_t *pgtab;

    pde = &pgdir[PDX(va)];

    if(*pde & PTE_P){
        pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
    } else {
        if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
            return 0;
        // Make sure all those PTE_P bits are zero.
        memset(pgtab, 0, PGSIZE);
        // The permissions here are overly generous, but they can
        // be further restricted by the permissions in the page table
        // entries, if necessary.
        /* PTE_W is especially needed for the region where the kernel stack is
           going to live! */
        *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
    }
    return &pgtab[PTX(va)];
}

// For debugging
uint32_t paging_get_paddr(uint32_t vaddr) {
    pte_t *pte = walkpgdir(kpgdir, vaddr, false);
    assert(pte != NULL);
    return *pte + ADDR_PAGE_OFFSET(vaddr);
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
static int
mappages(pde_t *pgdir, uintptr_t va, uint32_t size, uint32_t pa, int perm)
{
  char *a, *last;
  pte_t *pte;

  a = (char*)PGROUNDDOWN((uint32_t)va);
  last = (char*)PGROUNDDOWN(((uint32_t)va) + size - 1);
  for(;;){
    if((pte = walkpgdir(pgdir, (uint32_t)a, true)) == 0)
      return -1;

    if(*pte & PTE_P)
      panic("remap");

    *pte = pa | perm | PTE_P;

    if(a == last)
      break;

    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

/** Page fault (ISR # 14) handler. */
static void page_fault_handler(struct interrupt_state *state) {
    (void) state;   /** Unused. */

    /** The CR2 register holds the faulty address. */
    uint32_t faulty_addr;
    __asm__ ( "movl %%cr2, %0" : "=r" (faulty_addr) : );

    /**
     * Analyze the least significant 3 bits of error code to see what
     * triggered this page fault:
     *   - bit 0: page present -> 1, otherwise 0
     *   - bit 1: is a write operation -> 1, read -> 0
     *   - bit 2: is from user mode -> 1, kernel -> 0
     *
     * See https://wiki.osdev.org/Paging for more.
     */
    bool present = state->err_code & 0x1;
    bool write   = state->err_code & 0x2;
    bool user    = state->err_code & 0x4;

    /** Just prints an information message for now. */
    info("Caught page fault {\n"
         "  faulty addr = %p\n"
         "  present: %d\n"
         "  write:   %d\n"
         "  user:    %d\n"
         "}", faulty_addr, present, write, user);

    panic("page fault not handled!");
}

/** Switch the current page directory to the given one. */
inline void paging_switch_pgdir(const pde_t *pgdir) {
    assert(pgdir != NULL);
    __asm__ __volatile__ ( "movl %0, %%cr3" : : "r" (pgdir) );
}

/** Initialize paging and switch to use paging. */
void paging_init(uint64_t phys_end) {
    /**
     * Allocate the one-page space for the kernel's page directory in
     * the kernel heap. All pages of page directory/tables must be
     * page-aligned.
     */
    if((kpgdir = (pde_t*)kalloc()) == 0)
        panic("kalloc");
    memset(kpgdir, 0, PGSIZE);

    /**
     * Map all physical memory to the kernel's virtual address space.
     *
     * See also xv6 kernel's mappings:
     * https://github.com/mit-pdos/xv6-public/blob/eeb7b415dbcb12cc362d0783e41c3d1f44066b17/vm.c#L105
     */
    if(mappages(kpgdir, KERNBASE, phys_end - 0, 0, PTE_W) < 0) {
        /* freevm(pgdir); */
        panic("mappages");
    }

    /**
     * Register the page fault handler. This action must be done before
     * we do the acatual switch towards using paging.
     */
    isr_register(INT_IRQ_PAGE_FAULT, &page_fault_handler);

    /** Load the address of kernel page directory into CR3. */
    paging_switch_pgdir((pde_t*)V2P(kpgdir));
}
