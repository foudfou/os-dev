/**
 * Common debugging utilities.
 *
 * Lifted from https://github.com/josehu07/hux-kernel
 */


#ifndef DEBUG_H
#define DEBUG_H

#include "drivers/vga.h"
#include "drivers/screen.h"


/** Panicking macro. */
#define panic(fmt, args...) do {                                        \
        __asm__ __volatile__( "cli" );                                  \
        cprintf("PANIC: " fmt "\n", ##args);                            \
        while (1)                                                       \
            __asm__ __volatile__( "hlt" );                              \
        __builtin_unreachable();                                        \
    } while (0)

/** Assertion macro. */
#define assert(condition)   do {                                        \
        if (!(condition)) {                                             \
            panic("assertion failed @ function '%s',"                   \
                  " file '%s': line %d",                                \
                  __FUNCTION__, __FILE__, __LINE__);                    \
        }                                                               \
    } while (0)

/** Error prompting macro. */
#define error(fmt, args...) do {                                        \
        cprintf("ERROR: " fmt "\n", ##args);                            \
        panic("error occurred @ function '%s',"                         \
              " file '%s': line %d",                                    \
              __FUNCTION__, __FILE__, __LINE__);                        \
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
