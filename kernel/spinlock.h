#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

// Mutual exclusion lock.
struct spinlock {
  // Using uint32_t instead of bool for xchg.
  uint32_t locked;   // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
  uint32_t pcs[10];  // The call stack (an array of program counters)
                     // that locked the lock.
};


static inline uint32_t
readeflags(void)
{
  uint32_t eflags;
  __asm__ __volatile__("pushfl; popl %0" : "=r" (eflags));
  return eflags;
}

static inline void
cli(void)
{
  __asm__ __volatile__("cli");
}

static inline void
sti(void)
{
  __asm__ __volatile__("sti");
}

static inline uint32_t
xchg(volatile uint32_t *addr, uint32_t newval)
{
  uint32_t result;

  // The + in "+m" denotes a read-modify-write operand.
  __asm__ __volatile__(
      "lock; xchgl %0, %1" :
      // https://forum.osdev.org/viewtopic.php?f=1&t=33690
      "+m" (*addr), "=r" (result) : // %0 memory location read and written
                                    // to. %1 is any register and is placed
                                    // into the variable `result`.
      "1" (newval) : // before the instruction, the register corresponding to
                     // %1 shall be filled with the content of `newval`.
      /* "cc" // inform the compiler that condition codes in eflags wll change */
      /*      // during the operation (which is not true for xchg), so it shall */
      /*      // not rely on them being preserved. */
      );
  return result;
}


void initlock(struct spinlock *lk, char *name);
void acquire(struct spinlock *lk);
void release(struct spinlock *lk);

void getcallerpcs(void *v, uint32_t pcs[]);
int holding(struct spinlock *lock);

void pushcli(void);
void popcli(void);


#endif /* SPINLOCK_H */
