#include "cpu/exec.h"
#include "cpu/cc.h"

make_EHelper(test) {
  // TODO();
  // The flags SF, ZF, PF are modified while the result of the AND is discarded
  rtl_and(&t2, &id_dest->val, &id_src->val);
  t0 = 0;
  rtl_set_OF(&t0);
  rtl_set_CF(&t0);
  // AF
  rtl_update_ZFSF(&t2, decoding.dest.width);
  print_asm_template2(test);
}

make_EHelper(and) {
  // TODO();
  rtl_and(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);

  t0 = 0;
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
  rtl_update_ZFSF(&id_dest->val, decoding.dest.width);
  // AF
  
  print_asm_template2(and);
}

make_EHelper(xor) {
  // WARN I can not understand decode throughly, this following maybe possible two
  // try it !
  // rtl_xor(&id_src->val, &id_src->val, &id_dest->val);
  // TODO();
  rtl_xor(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);

  t0 = 0;
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
  rtl_update_ZFSF(&id_dest->val, decoding.dest.width);

  print_asm_template2(xor);
}

make_EHelper(or) {
  // TODO();
  rtl_or(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);

  t0 = 0;
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
  rtl_update_ZFSF(&id_dest->val, decoding.dest.width);

  print_asm_template2(or);
}

make_EHelper(sar) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  id_dest->val = ((int)id_dest->val) >> (id_src-> val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  operand_write(id_dest, &id_dest->val);
  print_asm_template2(sar);
}

make_EHelper(shl) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  id_dest->val = id_dest->val <<  (id_src-> val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  operand_write(id_dest, &id_dest->val);
  print_asm_template2(shl);
}


make_EHelper(shr) {
  // TODO();
  // unnecessary to update CF and OF in NEMU
  id_dest->val = (uint32_t)id_dest->val >>  (id_src-> val);
  rtl_update_ZFSF(&id_dest->val, id_dest->width);
  operand_write(id_dest, &id_dest->val);
  print_asm_template2(shr);
}


make_EHelper(ror){
  int shift = id_src->val;
  shift &= 0x1f;
  if(shift == 1){
    printf("warning OF is not handled\n");
  }
  unsigned int res = id_dest->val;
  for (int i = 0; i < shift; ++i) {
    int last_bit = res & 1;
    res = res >> 1;
    if(last_bit){
      cpu.CF = 1;
      res |= (1 << 31);
    }else{
      cpu.CF = 0;
    }
  }
  rtl_update_ZFSF(&res, id_dest->width);
  operand_write(id_dest, &res);
  print_asm_template2(ror);
}

make_EHelper(rcr){
  assert(0);
}

make_EHelper(rcl){
  assert(0);
}

make_EHelper(rol){
  int shift = id_src->val;
  shift &= 0x1f;
  if(shift == 1){
    printf("warning OF is not handled\n");
  }
  unsigned int res = id_dest->val;
  for (int i = 0; i < shift; ++i) {
    int first_bit = res & (1 << 31);
    res = res << 1;
    if(first_bit){
      cpu.CF = 1;
      res |= 1;
    }else{
      cpu.CF = 0;
    }
  }
  rtl_update_ZFSF(&res, id_dest->width);
  operand_write(id_dest, &res);
  print_asm_template2(rol);
}


// setcc : read a eflags into memory or reg
make_EHelper(setcc) {
  uint32_t cc = decoding.opcode & 0xf; // there are only sixteen
  rtl_setcc(&t2, cc);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
  // TODO();
  rtl_not(&id_dest->val, &id_dest->val);
  operand_write(id_dest, &id_dest->val);
  // Flags Affected None
  print_asm_template1(not);
}
