// Mutual exclusion spin locks.

#include "drivers/screen.h"
#include "cpu.h"
#include "paging.h"
#include "proc.h"

#include "spinlock.h"



void
initlock(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->cpu = 0;
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void
acquire(struct spinlock *lk)
{
  pushcli(); // disable interrupts to avoid deadlock.
  if(holding(lk))
    panic("acquire");

  // The xchg is atomic.
  while(xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();

  // Record info about lock acquisition for debugging.
  lk->cpu = mycpu();
  getcallerpcs(&lk, lk->pcs);
}

// Release the lock.
void
release(struct spinlock *lk)
{
  if(!holding(lk))
    panic("release");

  lk->pcs[0] = 0;
  lk->cpu = 0;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other cores before the lock is released.
  // Both the C compiler and the hardware may re-order loads and
  // stores; __sync_synchronize() tells them both not to.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code can't use a C assignment, since it might
  // not be atomic. A real OS would use C atomics here.
  __asm__ __volatile__("movl $0, %0" : "+m" (lk->locked) : );

  popcli();
}

/**
 * Establishes call stack with max depth 10.
 *
 * We can generate a call chain by iterating on each frame's ebp+1 which
 * contains the return address, that is the EIP value of the jump instruction
 * in the caller.
 *
 * Note this version is slightly more precise than hux's stack_trace() as it
 * records EIPs instead of EBPs.
 *
 *
 * We could theoretically also fetch the function names from the ELF symbol
 * table, by finding the code section that is a function and contains the saved
 * EIP value. See lookup function in hux:
 * https://github.com/josehu07/hux-kernel/blob/main/src/common/debug.c#L34
 */
// Record the current call stack in pcs[] by following the %ebp chain.
void
getcallerpcs(void *v, uint32_t pcs[])
{
  uint32_t *ebp;
  int i;

  ebp = (uint32_t*)v - 2;
  for(i = 0; i < 10; i++){
    if(ebp == 0 || ebp < (uint32_t*)KERNBASE || ebp == (uint32_t*)0xffffffff)
      break;
    pcs[i] = ebp[1];     // saved %eip
    ebp = (uint32_t*)ebp[0]; // saved %ebp
  }
  for(; i < 10; i++)
    pcs[i] = 0;
}

// Check whether this cpu is holding the lock.
int
holding(struct spinlock *lock)
{
  pushcli();
  bool r = lock->locked && lock->cpu == mycpu();
  popcli();
  return r;
}


// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.

void
pushcli(void)
{
  int eflags = readeflags();
  cli();
  if(mycpu()->ncli == 0)
    mycpu()->intena = eflags & FL_IF;
  mycpu()->ncli += 1;
}

void
popcli(void)
{
  if(readeflags() & FL_IF)
    panic("popcli - interruptible");
  if(--mycpu()->ncli < 0)
    panic("popcli");
  if(mycpu()->ncli == 0 && mycpu()->intena)
    sti();
}
