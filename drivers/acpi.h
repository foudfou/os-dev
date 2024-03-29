// Lifted from https://github.com/thomasloven/mittos64/blob/8fac92474adb88a1deca15a34b68b12658dfd5ce/src/kernel/include/acpi.h
#ifndef ACPI_H
#define ACPI_H

#include <stdint.h>

#define MAX_CPUS 16
#define MAX_IOAPIC 4

extern uintptr_t acpi;
extern uint32_t  acpi_len;

struct acpi_info {
  int num_cpus;

  struct {
    uint8_t id;
    uint8_t apic;
  } cpu[MAX_CPUS];

  int num_ioapic;

  struct {
    uint8_t  id;
    uint32_t addr;
    uint32_t base; // first interrupt number
  } ioapic[MAX_IOAPIC];

  uint32_t int_map[256];
};

extern struct acpi_info acpi_info;

void acpi_init();

#endif /* ACPI_H */
