#include "drivers/screen.h"
#include "kernel/idt.h"

void main () {
  clear_screen();
  print("FOUDIL WAS HERE\n(c) 2022\n");

  idt_init();

  // Just to illustrate interrupts
  __asm__("int $0x05");
}
