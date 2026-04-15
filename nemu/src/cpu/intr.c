#include "cpu/exec.h"

#include "memory/mmu.h"



void raise_intr(uint8_t NO, vaddr_t ret_addr) {

  rtl_push(&cpu.eflags.val);

  rtl_push(&ret_addr);



  uint32_t addr = cpu.idtr.base + NO * 8;

  GateDesc gate;

  gate.val = vaddr_read(addr, 4);

  gate.val |= ((uint64_t)vaddr_read(addr + 4, 4)) << 32;



  decoding.jmp_eip = (gate.offset_31_16 << 16) | gate.offset_15_0;

  decoding.is_jmp = 1;

}



void dev_raise_intr() {

}
