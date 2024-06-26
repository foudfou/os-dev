#include "lib/debug.h"
#include "paging.h"
#include "lib/string.h"

#include "drivers/acpi.h"

#define incptr(p, n) ((void *)(((uintptr_t)(p)) + (n)))

#define ACPI_RSDP_CHECKSUM_LENGTH       20
#define ACPI_RSDP_XCHECKSUM_LENGTH      36

#define MADT_CPU    0
#define MADT_IOAPIC 1
#define MADT_INT    2

uintptr_t acpi = 0;
uint32_t  acpi_len = 0;

struct rsdp {
  uint8_t  signature[8];
  uint8_t  checksum;
  uint8_t  OEMID[6];
  uint8_t  revision;
  uint32_t rsdt;
  uint32_t length;
  uint64_t xsdt;
  uint8_t  checksum2;
  uint8_t  _[3];
} __attribute__((packed));
#define RSDP_SIGNATURE "RSD PTR "

struct sdt {
  uint8_t  signature[4];
  uint32_t len;
  uint8_t  revision;
  uint8_t  checksum;
  uint8_t  OEMID[6];
  uint8_t  table_ID[8];
  uint32_t OEM_revision;
  uint32_t creator;
  uint32_t creator_rev;
  uint8_t  data[];
} __attribute__((packed));
#define MADT_SIGNATURE "APIC"

struct madt {
  uint32_t lic_address;
  uint32_t flags;
  uint8_t  data[];
} __attribute__((packed));

struct madt_entry
{
  uint8_t      type;
  uint8_t      len;
  union{
    struct {
      uint8_t  id;
      uint8_t  apic;
      uint32_t flags;
    } __attribute__((packed)) lapic;
    struct {
      uint8_t  id;
      uint8_t  _;
      uint32_t addr;
      uint32_t base;
    } __attribute__((packed)) ioapic;
    struct {
      uint8_t  bus;
      uint8_t  source;
      uint32_t target;
      uint16_t flags;
    } __attribute__((packed)) interrupt;
  };
} __attribute__((packed));

struct acpi_info acpi_info = {0};

static void *scan_rsdp(uint32_t start, uint32_t end)
{
  void *p = P2V(start);
  while(p < P2V(end))
  {
    if(!memcmp(p, RSDP_SIGNATURE, 8))
      return p;
    p = incptr(p, 16);
  }
  return 0;
}

static struct rsdp *find_rsdp()
{
  // Scan the Extended BIOS Data Area
  uint16_t *ebda_ptr = P2V(0x40e);
  uint64_t ebda = *ebda_ptr << 4;
  void *p = scan_rsdp(ebda, ebda+1024);
  if(p) return p;

  // Scan 0xE0000 - 0xFFFFF
  p = scan_rsdp(0xE0000, 0xFFFFF);
  if(p) return p;

  return 0;
}

static void parse_madt(struct madt *madt, uint32_t len)
{
  uintptr_t end = (uintptr_t)madt + len;
  struct madt_entry *e = (void *)madt->data;
  cprintf("Local Interrupt Controller: 0x%x\n", madt->lic_address);
  while((uintptr_t)e < end)
  {
    if (e->len == 0) // guard rail since bochs reports strange entries…
      break;

    int i;
    switch(e->type)
    {
      case MADT_CPU: // APIC descriptor (corresponds to unique cpu core)
        // Check if cpu is enabled
        if(!(e->lapic.id & 1)) break;
        // Add to list
        i = acpi_info.num_cpus;
        acpi_info.cpu[i].id = e->lapic.id;
        acpi_info.cpu[i].apic = e->lapic.apic;
        acpi_info.num_cpus++;
        break;

      case MADT_IOAPIC: // IOAPIC descriptor
        i = acpi_info.num_ioapic;
        acpi_info.ioapic[i].id = e->ioapic.id;
        acpi_info.ioapic[i].addr = e->ioapic.addr;
        acpi_info.ioapic[i].base = e->ioapic.base;
        acpi_info.num_ioapic++;
        break;

      case MADT_INT: // Interrupt remap
        acpi_info.int_map[e->interrupt.source] = e->interrupt.target;
        break;
    }
    /* cprintf(" MADT: type:%d len:%d\n", e->type, e->len); */
    e = incptr(e, e->len);
  }
}

static void parse_sdt(struct sdt *sdt, uint8_t revision)
{
  uint32_t *p32 = (void *)sdt->data;
  uint64_t *p64 = (void *)sdt->data;
  int entries = (sdt->len - sizeof(struct sdt)) / (revision ? 8 : 4);
  for(int i = 0; i < entries; i++)
  {
    struct sdt *table = P2V(revision ? p64[i] : p32[i]);
    /* cprintf("Found table: %x\n", (char*)table->signature); */
    if(!memcmp(table->signature, MADT_SIGNATURE, 4))
      parse_madt((void *)table->data, table->len);
  }
}

static uint8_t compute_checksum(uint8_t *buffer, uint32_t length)
{
    uint8_t *end = buffer + length;
    uint8_t sum = 0;

    while (buffer < end)
        sum += *(buffer++);

    return sum;
}

void acpi_init()
{
  struct rsdp *rsdp = find_rsdp();
  struct sdt *s = P2V(rsdp->revision ? rsdp->xsdt : rsdp->rsdt);
  if (compute_checksum((uint8_t *)rsdp, ACPI_RSDP_CHECKSUM_LENGTH) != 0) {
      warn("Invalid RSDP checksum");
      // FIXME assume default values here
      return;
  }
  parse_sdt(s, rsdp->revision);
}
