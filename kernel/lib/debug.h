/**
 * Common debugging utilities.
 *
 * Lifted from https://github.com/josehu07/hux-kernel
 */


#ifndef DEBUG_H
#define DEBUG_H

#include "drivers/vga.h"
#include "drivers/screen.h"

void stack_trace();

/** Assertion macro. */
#define assert(condition)   do {                                        \
        if (!(condition)) {                                             \
            cprintf("function '%s', file '%s': line %d\n",              \
                    __FUNCTION__, __FILE__, __LINE__);                  \
            panic("assertion failed");                                  \
        }                                                               \
    } while (0)

/** Error prompting macro. */
#define error(fmt, args...) do {                                        \
        cprintf("ERROR: " fmt "\n", ##args);                            \
        cprintf("function '%s', file '%s': line %d\n",                  \
                __FUNCTION__, __FILE__, __LINE__);                      \
        panic("error occurred");                                        \
    } while (0)

/** Warning prompting macro. */
#define warn(fmt, args...)  do {                                        \
        cprintf("WARN: " fmt "\n", ##args);                             \
    } while (0)

/** Info prompting macro. */
#define info(fmt, args...)  do {                                        \
        cprintf("INFO: " fmt "\n", ##args);                             \
    } while (0)


#endif
