// The I/O APIC manages hardware interrupts for an SMP system.
// http://www.intel.com/design/chipsets/datashts/29056601.pdf
// See also picirq.c.

#include "drivers/acpi.h"
#include "drivers/screen.h"
#include "idt.h"
#include "paging.h"
#include "lib/debug.h"

#include "drivers/ioapic.h"

#define REG_ID     0x00  // Register index: ID
#define REG_VER    0x01  // Register index: version
#define REG_TABLE  0x10  // Redirection table base

// The redirection table starts at REG_TABLE and uses
// two registers to configure each interrupt.
// The first (low) register in a pair contains configuration bits.
// The second (high) register contains a bitmask telling which
// CPUs can serve that interrupt.
#define INT_DISABLED   0x00010000  // Interrupt disabled
#define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
#define INT_ACTIVELOW  0x00002000  // Active low (vs high)
#define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)

volatile struct ioapic *ioapic;

// IO APIC MMIO structure: write reg, then read or write data.
struct ioapic {
  uint32_t reg;
  uint32_t pad[3];
  uint32_t data;
};

static uint32_t
ioapicread(uint32_t reg)
{
  ioapic->reg = reg;
  return ioapic->data;
}

static void
ioapicwrite(uint32_t reg, uint32_t data)
{
  ioapic->reg = reg;
  ioapic->data = data;
}

void
ioapicinit(void)
{
  // Assume SMP system with single IOAPIC for now
  assert(acpi_info.num_ioapic == 1);

  ioapic = (volatile struct ioapic*)acpi_info.ioapic[0].addr;
  uint32_t maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
  uint32_t id = ioapicread(REG_ID) >> 24;
  cprintf("IOAPIC: ioapic_id=%d, acpi_ioapicid=%d, address=0x%p, maxintr=%d\n",
          id, acpi_info.ioapic[0].id, acpi_info.ioapic[0].addr, maxintr);
  if(id != acpi_info.ioapic[0].id)
    cprintf("ioapicinit: id isn't equal to ioapicid; not a MP\n");

  // Mark all interrupts edge-triggered, active high, disabled,
  // and not routed to any CPUs.
  for(uint32_t i = 0; i <= maxintr; i++){
    ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (IDT_IRQ_BASE + i));
    ioapicwrite(REG_TABLE+2*i+1, 0);
  }
}

void
ioapicenable(uint32_t irq, uint32_t cpunum)
{
  // Mark interrupt edge-triggered, active high,
  // enabled, and routed to the given cpunum,
  // which happens to be that cpu's APIC ID.
  ioapicwrite(REG_TABLE+2*irq, IDT_IRQ_BASE + irq);
  ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
}
