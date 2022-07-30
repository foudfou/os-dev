#include "lib/debug.h"
#include "lib/string.h"
#include "kernel/idt.h"
#include "kernel/pmem.h"

#include "kernel/paging.h"

/** kernel's identity-mapping page directory. */
pde_t *kernel_pgdir;    /** Allocated at paging init. */


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
        if(!alloc)
            return 0;

        pgtab = (pte_t *) kalloc_temp(PAGE_SIZE, true);
        assert(pgtab != NULL);
        memset(pgtab, 0, PAGE_SIZE);

        *pde = V2P(pgtab) | PTE_P | PTE_U;
    }
    return &pgtab[PTX(va)];
}

// For debugging
uint32_t paging_get_paddr(uint32_t vaddr) {
    pte_t *pte = walkpgdir(kernel_pgdir, vaddr, false);
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
void paging_init(uint64_t ram_size) {
    /**
     * Allocate the one-page space for the kernel's page directory in
     * the kernel heap. All pages of page directory/tables must be
     * page-aligned.
     */
    kernel_pgdir = (pde_t *) kalloc_temp(PAGE_SIZE, true);
    memset(kernel_pgdir, 0, PAGE_SIZE);

    /**
     * Identity-map the kernel's virtual address space to the physical
     * memory. This means we need to map all the allowed kernel physical frames
     * (0 -> KHEAP_MAX_ADDR) as its identity virtual address in the kernel page
     * table, and reserve this entire physical memory region.
     */
    uint32_t addr = 0;
    while (addr < KHEAP_MAX_ADDR) {
        uint64_t frame_idx = frame_alloc();
        assert(frame_idx < num_frames);

        pte_t *pte = walkpgdir(kernel_pgdir, addr, true);
        assert(pte != NULL);

        *pte = (frame_idx * PAGE_SIZE) | PTE_P;

        addr += PAGE_SIZE;
    }

    /**
     * Also map the rest of physical memory just so the kernel can access any
     * physical address directly. We thus mark pte's present. We don't allocate
     * frames for now though, which is inconsistent, but can will this later.
     *
     * See also paging mapping for xv6
     * https://github.com/mit-pdos/xv6-public/blob/master/vm.c#L105
     */
    while (addr < ram_size) {
        pte_t *pte = walkpgdir(kernel_pgdir, addr, true);
        assert(pte != NULL);

        *pte = ADDR_PAGE_NUMBER(addr) | PTE_P;
        /* cprintf("__*pte=%p, addr=%p\n", *pte, addr); */

        addr += PAGE_SIZE;
    }

    /**
     * Register the page fault handler. This acation must be done before
     * we do the acatual switch towards using paging.
     */
    isr_register(INT_IRQ_PAGE_FAULT, &page_fault_handler);

    /** Load the address of kernel page directory into CR3. */
    paging_switch_pgdir(kernel_pgdir);

    /**
     * Enable paging by setting the two proper bits of CR0:
     *   - PG bit (31): enable paging
     *   - PE bit (0): enable protected mode
     *
     * We are not setting the WP bit, so the read/write bit of any PTE just
     * controls whether the page is user writable - in kernel priviledge any
     * page can be written.
     */
    uint32_t cr0;
    __asm__ __volatile__ ("movl %%cr0, %0" : "=r" (cr0) :);
    cr0 |= CR0_PG | CR0_PE;
    __asm__ __volatile__ ("movl %0, %%cr0" : : "r" (cr0));
}
