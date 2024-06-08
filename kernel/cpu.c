#include "drivers/screen.h"
#include "spinlock.h"
#include "lib/debug.h"

#include "cpu.h"

struct cpu cpus[MAX_CPUS];
int ncpu;
uint8_t ioapicid;

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");

  return &cpus[0];
}


void cpu_init() {
    cprintf("CPUS: %d\n", acpi_info.num_cpus);
    for (int i = 0; i < acpi_info.num_cpus; ++i) {
        cpus[i].apicid = acpi_info.cpu[i].apic;
    }
}
