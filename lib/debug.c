/**
 * Common debugging utilities.
 *
 * Lifted from https://github.com/josehu07/hux-kernel
 */


#include <stdint.h>
#include <stddef.h>
#include "drivers/screen.h"

#include "lib/debug.h"

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
