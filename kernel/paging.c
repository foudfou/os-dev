#include "drivers/acpi.h"
#include "drivers/screen.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "lib/utils.h"
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
    warn("Caught page fault {\n"
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

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
void
inituvm(pde_t *pgdir, char *init, size_t sz)
{
  if(sz >= PGSIZE)
    panic("inituvm: more than a page");
  char *mem = kalloc();
  memset(mem, 0, PGSIZE);
  // process virtual address space starts at 0x0.
  mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W|PTE_U);
  // We could use memcpy() instead of memmove() as I can't imagine any cases
  // where the newly allocated page would overlap with the init code.
  //
  // One reason to actually copy and not just map is that the init code doesn't
  // reside on a page boundary.
  memmove(mem, init, sz);
}


// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
uint32_t
deallocuvm(pde_t *pgdir, uint32_t oldsz, uint32_t newsz)
{
  pte_t *pte;
  uint32_t a, pa;

  if(newsz >= oldsz)
    return oldsz;

  a = PGROUNDUP(newsz);
  for(; a  < oldsz; a += PGSIZE){
    pte = walkpgdir(pgdir, a, 0);
    if(!pte) // pde not present
      a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
    else if(*pte & PTE_P){
      pa = PTE_ADDR(*pte);
      if(pa == 0)
        panic("kfree");
      char *v = P2V(pa);
      kfree(v);
      *pte = 0;
    }
  }
  return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
void
freevm(pde_t *pgdir)
{
  if(pgdir == 0)
      panic("freevm: no pgdir");

  // Frames to be freed: page directory, page tables, user process code. But
  // user code is only referenced/reachable by virtual address 0x (and up),
  // hence deallocuvm().
  deallocuvm(pgdir, KERNBASE, 0);

  for(uint32_t i = 0; i < NPDENTRIES; i++){
    if(pgdir[i] & PTE_P){
      char * pte = P2V(PTE_ADDR(pgdir[i]));
      kfree(pte);
    }
  }

  kfree((char*)pgdir);
}

// Allocate a page table and initialize its kernel part.
pde_t*
setupkvm(void)
{
    pde_t *pgdir;

    /**
     * Allocate the one-page space for the kernel's page directory in
     * the kernel heap. All pages of page directory/tables must be
     * page-aligned.
     */
    if((pgdir = (pde_t*)kalloc()) == 0)
        panic("kalloc");
    memset(pgdir, 0, PGSIZE);

    /**
     * Map all physical memory to the kernel's virtual address space.
     *
     * See also xv6 kernel's mappings:
     * https://github.com/mit-pdos/xv6-public/blob/eeb7b415dbcb12cc362d0783e41c3d1f44066b17/vm.c#L105
     */
    if(mappages(pgdir, KERNBASE, phys_end - 0, 0, PTE_W) < 0)
        goto mapfail;

    // For ACPI. The should not be any overlap with kernel space. Size
    // arbitrarily set to the whole space until virtual start of virtual kernel
    // space.
    if(mappages(pgdir, (uintptr_t)P2V(acpi), acpi_len, acpi, PTE_W) < 0)
        goto mapfail;

    // For IOAPIC. FIXME why if 0xfec00000 not in the pmem tables???
    if(mappages(pgdir, DEVSPACE, 0xf00000, DEVSPACE, PTE_W) < 0)
        goto mapfail;

    return pgdir;

  mapfail:
    freevm(pgdir);
    panic("mappages");
}


/** Initialize paging and switch to use paging. */
void paging_init()
{
    kpgdir = setupkvm();

    /**
     * Register the page fault handler. This action must be done before
     * we do the acatual switch towards using paging.
     */
    isr_register(IDT_INT_PGFLT, &page_fault_handler);

    /** Load the address of kernel page directory into CR3. */
    paging_switch_pgdir((pde_t*)V2P(kpgdir));
}
