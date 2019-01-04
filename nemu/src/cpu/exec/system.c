#include "cpu/exec.h"
#include "device/port-io.h"
#include "common.h"

void difftest_skip_ref();
void difftest_skip_dut();

uint16_t idtr_size;
uint32_t idtr_addr;

make_EHelper(lidt) {
  // TODO();
  idtr_size = vaddr_read(id_dest->addr, 2);
  idtr_addr = vaddr_read(id_dest->addr + 2, 4); // 
  print_asm_template1(lidt);
}


void read_ModR_M(vaddr_t *eip, Operand *rm, bool load_rm_val, Operand *reg, bool load_reg_val);

make_EHelper(mov_r2cr) {
  // TODO();
  read_ModR_M(eip, id_src, true, id_dest, false);
  assert(id_src->type == OP_TYPE_REG);
  assert(id_dest->type == OP_TYPE_REG);

  // change the GPR to crt 
  switch (id_dest->reg) {
    case 3:
      cpu.cr3.val = id_src->val;
      break;
    case 0:
      cpu.cr0.val = id_src->val;
      break;
    default:
      panic("Impossible, only two crt0 implement !");
  }
  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  // id_src ---> eax
  // id_dest ---> cr3
  read_ModR_M(eip, id_src, false, id_dest, false);
  assert(id_src->type == OP_TYPE_REG);
  assert(id_dest->type == OP_TYPE_REG);

  // change the GPR to crt 
  switch (id_dest->reg) {
    case 3:
      id_src->val = cpu.cr3.val;
      break;
    case 0:
      id_src->val = cpu.cr0.val;
      break;
    default:
      panic("Impossible, only two crt0 implement !");
  }

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));
#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

void raise_intr(uint8_t NO, vaddr_t ret_addr);
make_EHelper(int) {
  // TODO();
  // 依次将EFLAGS, CS(代码段寄存器), EIP寄存器的值压栈
  // printf("We don't save eflags, cs, eip\n");
  
  
  // 从IDTR中读出IDT的首地址
  // 根据异常号在IDT中进行索引, 找到一个门描述符
  // 将门描述符中的offset域组合成目标地址
  // 跳转到目标地址

  uint8_t int_NO = id_dest->val;
  raise_intr(int_NO, *eip);


  print_asm("int %s", id_dest->str);

#if defined(DIFF_TEST) && defined(DIFF_TEST_QEMU)
  difftest_skip_dut();
#endif
}

make_EHelper(iret) {
  // rtl_pop(eip);

  rtl_pop(&t0);
  rtl_pop(&cpu.cs);
  rtl_pop(&cpu.eflags);

  // assert(cpu.IF);

  rtl_jr(&t0);
  print_asm("iret");
}

// 实现in, out指令, 在它们的helper函数中分别调用 pio_read_[l|w|b]()和pio_write_[l|w|b]
// in move data to e[ax]
make_EHelper(in) {
  // TODO();
  if(id_dest->width == 1){
    t0 = pio_read_b(id_src->val);
  }else if(id_dest->width == 4){
    t0 = pio_read_l(id_src->val);
  }else if(id_dest->width == 2){
    t0 = pio_read_w(id_src->val);
  }else{
    panic("exec_in id_dest->width is invalid!");
  }
  operand_write(id_dest, &t0);

  print_asm_template2(in);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(out) {
  // TODO();
  if(id_dest->width == 1){
    pio_write_b(id_dest->val, id_src->val);
  }else if(id_dest->width == 4){
    pio_write_w(id_dest->val, id_src->val);
  }else if(id_dest->width == 2){
    pio_write_l(id_dest->val, id_src->val);
  }else{
    panic("exec_out id_dest->width is invalid!");
  }

  print_asm_template2(out);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}
