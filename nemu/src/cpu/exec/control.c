#include "cpu/exec.h"
#include "cpu/cc.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  rtl_j(decoding.jmp_eip);

  print_asm("jmp %x", decoding.jmp_eip);
}

// for all kinds of conditional jump, we can refer jcc
make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint32_t cc = decoding.opcode & 0xf;
  rtl_setcc(&t0, cc);
  rtl_li(&t1, 0);
  rtl_jrelop(RELOP_NE, &t0, &t1, decoding.jmp_eip);

  print_asm("j%s %x", get_cc_name(cc), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  // WARN rtl_jr and rtl_j, what's the difference between them ?
  rtl_jr(&id_dest->val);

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  // TODO();
  // Push(EIP);
  // EIP ← EIP + rel32;
  // printf("exec with call\n");
  // eip = eip + 4;
  rtl_push(eip);
  rtl_j(decoding.jmp_eip);
  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  // TODO();
  // rtl_pop(eip);
  rtl_pop(&t0);
  rtl_j(t0);
  print_asm("ret");
}

make_EHelper(retn) {
  // TODO();
  // rtl_pop(eip);
  rtl_pop(&t0);
  rtl_j(t0);
  cpu.esp += id_dest->val;
  print_asm("retn");
}

make_EHelper(call_rm) {
  // TODO();
  rtl_push(eip);
  rtl_jr(&id_dest->val);

  print_asm("call *%s", id_dest->str);
}
