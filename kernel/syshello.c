#include "drivers/screen.h"
#include "syscall.h"

int sys_hello(void) {
    struct process *proc = myproc();

    int32_t i;
    char *p, *str;

    if (sysarg_get_int(proc, 0, &i) < 0)
        return SYSFAIL;
    if (sysarg_get_ptr(proc, 1, &p, 8) < 0)
        return SYSFAIL;
    int len = sysarg_get_str(proc, 2, &str);
    if (len < 0)
        return SYSFAIL;

    cprintf("Hello world! i=%d p=0x%x len=%d str=%s\n", i, p, len, str);

    return 0;
}
