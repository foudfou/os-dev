#include "lib/debug.h"
#include "lib/utils.h"

#include "syscall.h"

extern int sys_hello(void);
extern int sys_exit(void);

// For readability
typedef int (*syscall_fn)(void);

static syscall_fn syscalls[] = {
    [SYS_hello]   = sys_hello,
    /* [SYS_fork]   =  sys_fork, */
    [SYS_exit]    = sys_exit,
};

void syscall_handler(struct interrupt_state *state) {
    myproc()->tf = state;

    uint32_t num = state->eax;
    if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        state->eax = syscalls[num]();
    } else {
        warn("Unknown sys call %d\n", num);
        state->eax = SYSFAIL;
    }
}


// User code makes a system call with INT T_SYSCALL.
// System call number in %eax.
// Arguments on the stack, from the user call to the C
// library system call function. The saved user %esp points
// to a saved program counter, and then the first argument.

// n-th element on the stack pointer (p) after EIP
#define CDECL_ARG(p, n) (p) + sizeof(uint32_t)*(1 + (n))

// Fetch the int at addr from the current process.
static inline int
fetchint(struct process *proc, int n, int32_t *ip)
{
  uint32_t addr = CDECL_ARG(proc->tf->esp, n);
  if(addr >= proc->sz || addr+4 > proc->sz)
    return -1;
  *ip = *(int32_t*)(addr);
  return 0;
}

// Fetch the nth 32-bit system call argument.
int sysarg_get_int(struct process *proc, int n, int32_t *ip)
{
    return fetchint(proc, n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int sysarg_get_ptr(struct process *proc, int n, char **pp, size_t size)
{
  int32_t i;
  if(fetchint(proc, n, &i) < 0)
    return -1;
  if(size < 0 || (uint32_t)i >= proc->sz || (uint32_t)i+size > proc->sz)
    return -1;
  *pp = (char*)i;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int fetchstr(struct process *proc, uint32_t addr, char **pp)
{
  if(addr >= proc->sz)
    return -1;
  *pp = (char*)addr;
  char *ep = (char*)proc->sz;
  for(char *s = *pp; s < ep; s++){
    if(*s == '\0')
      {
      return s - *pp;
      }
  }
  return -1;
}

// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int sysarg_get_str(struct process *proc, int n, char **pp)
{
  int32_t addr;
  if(sysarg_get_int(proc, n, &addr) < 0)
    return -1;
  return fetchstr(proc, addr, pp);
}
