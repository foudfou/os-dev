#ifndef SYSCALL_H
#define SYSCALL_H

#include "kernel/idt.h"

// System call numbers
#define SYS_hello   1
/* #define SYS_fork    1 */
/* #define SYS_exit    2 */

#define SYSFAIL (-1)

void syscall_handler(struct interrupt_state *state);

int sysarg_get_int(int n, int32_t *ip);
int sysarg_get_ptr(int n, char **pp, size_t size);
int sysarg_get_str(int n, char **pp);

#endif /* SYSCALL_H */
