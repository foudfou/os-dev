/**
 * Common debugging utilities.
 *
 * Lifted from https://github.com/josehu07/hux-kernel
 */


#include <stdint.h>
#include <stddef.h>
#include "drivers/screen.h"

#include "lib/debug.h"

/** Virtual address space size: 1GiB. */
#define USER_MAX 0x40000000

/**
 * Print stack tracing to terminal.
 *
 * We can generate a call chain by iterating on each frame's ebp+1 which
 * contains the return address, that is the EIP value of the jump instruction
 * in the caller. FIXME complete
 */
void
stack_trace()
{
    uint32_t *ebp;
    unsigned int id = 0;

    __asm__ __volatile__("movl %%ebp, %0" : "=r" (ebp));
    while (ebp != NULL && (uint32_t) (ebp + 1) < USER_MAX) { // USER_MAX is the top of user address space stack
        uint32_t addr = *(ebp + 1);
        // Ideally we would print the function symbol name in addition to its
        // address. We can't do this currently because our kernel is still in
        // binary format and thus doesn't contain any symbol information. A
        // format like ELF would contain a symbol table. A bootloader should be
        // able to provide the kernel with a link to its ELF headers. But our
        // kernel doesn't currently expect and support any bootloader. We thus
        // can't build it ELF.
        cprintf(" %d) [%p]\n", id++, addr);
        ebp = (uint32_t *) *ebp;
    }
}

/**
 * Stack smashing protector (stack canary) support.
 * Build with `-fstack-protector` to enable this. Using a static canary
 * value here to maintain simplicity.
 */
#define STACK_CHK_GUARD 0xCF10A8CB

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

__attribute__((noreturn))
void
__stack_chk_fail(void)
{
    panic("stack smashing detected");
}
