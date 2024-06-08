#include "syscall.h"

int sys_exit(void) {
    struct process *proc = myproc();
    int32_t n;
    if (sysarg_get_int(proc, 0, &n) < 0)
        return SYSFAIL;
    exit(n);
    return 0;  // not reached
}
