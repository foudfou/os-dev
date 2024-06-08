#ifndef SYSCALL_H
#define SYSCALL_H

#include <stddef.h>

#include "idt.h"
#include "proc.h"

// System call numbers
#include "syscall_defs.h"

#define SYSFAIL (-1)

void syscall_handler(struct interrupt_state *state);

int sysarg_get_int(struct process *proc, int n, int32_t *ip);
int sysarg_get_ptr(struct process *proc, int n, char **pp, size_t size);
int sysarg_get_str(struct process *proc, int n, char **pp);

#endif /* SYSCALL_H */
