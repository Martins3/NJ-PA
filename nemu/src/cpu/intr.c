#include "cpu/exec.h"
#include "memory/mmu.h"


// define in the system.c
extern uint16_t idtr_size;
extern uint32_t idtr_addr;

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  // printf("make raise with IRQ number %d\n", NO);
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  // GateDesc * idt = (GateDesc *)idtr_addr;
  // printf("%x\n", idt);
  // printf("doing nothing %x\n", idtr_addr);

  rtl_push(&cpu.eflags);
  cpu.IF = 0; // assume we forbit every all IF in the interrupt
  rtl_push(&cpu.cs);
  rtl_push(&ret_addr); // eip

  // vaddr_t p = idtr_addr + sizeof(GateDesc) * NO;
  uint32_t a = vaddr_read(idtr_addr + sizeof(GateDesc) * NO, 4);
  uint32_t b = vaddr_read(idtr_addr + sizeof(GateDesc) * NO + 4, 4);
  uint32_t entry_addr = (a & 0xffff) | (b & 0xffff0000);

  // printf("we wanna jump to %x\n", entry_addr);

  rtl_j(entry_addr);

  // WARN 
  // why we should provide ret_addr
  // rtl_pop(&t0);
  // rtl_j(t0);
  // TODO();
}

// I can not understand why this is exec periodicly
void dev_raise_intr() {
  // cpu.INTR = true;
};
