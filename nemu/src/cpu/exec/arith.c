#include "cpu/exec.h"

/**
 *
0 CF Carry Flag ── Set on high-order bit carry(for add) or borrow(for substract); cleared otherwise.
2 PF Parity Flag ── Set if low-order eight bits of result contain an even number of 1 bits; cleared otherwise.
4 AF Adjust flag ── Set on carry from or borrow to the low order four bits of AL; cleared otherwise. Used for decimal arithmetic.
6 ZF Zero Flag ── Set if result is zero; cleared otherwise.
7 SF Sign Flag ── Set equal to high-order bit of result (0 is positive, 1 if negative).
11 OF Overflow Flag ── Set if result is too large a positive number or too small a negative number (excluding sign-bit) to fit in destination operand; cleared otherwise.

https://stackoverflow.com/questions/199333/how-to-detect-integer-overflow
 */

// #define make_EHelper(name) void concat(exec_, name) (vaddr_t *eip)
make_EHelper(add) {
  // TODO();
  
  rtlreg_t a = id_dest->val; 
  rtlreg_t b = id_src->val; 
  rtlreg_t one = 1;
  rtlreg_t zero = 0;
  rtlreg_t res = a + b;

  // WARN maybe cause problem, i don't check it 
  if(res < a || res < b){
    rtl_set_CF(&one);
  } else{
    rtl_set_CF(&zero);
  }

  rtl_add(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);

  rtl_update_ZFSF(&id_dest->val, decoding.dest.width);

// if ((x > 0) && (a > INT_MAX - x)) [> `a + x` would overflow <];
// if ((x < 0) && (a < INT_MIN - x)) [> `a + x` would underflow <];

  if ((((int32_t)b > 0) && ((int32_t)a > (int32_t)0x7fffffff - (int32_t)b)) ||
      (((int32_t)b < 0) && ((int32_t)a < (int32_t)0x80000000 - (int32_t)b))){
    rtl_set_OF(&one);
  }else{
    rtl_set_OF(&zero);
  }


  print_asm_template2(add);
}

make_EHelper(sub) {

  // TODO as for flags, not implemented yet
  // we need to understand 
  rtlreg_t a = id_dest->val; 
  rtlreg_t b = id_src->val; 
  rtlreg_t one = 1;
  rtlreg_t zero = 0;
  if(a < b){
    rtl_set_CF(&one);
  } else{
    rtl_set_CF(&zero);
  }

  // PF is useless yet
  // AF maybe useful, not now
  rtl_sub(&id_dest->val, &id_dest->val, &id_src->val);
  operand_write(id_dest, &id_dest->val);
  
  rtl_update_ZFSF(&id_dest->val, decoding.dest.width);

  // if ((x < 0) && (a > INT_MAX + x)) [> `a - x` would overflow <];
  // if ((x > 0) && (a < INT_MIN + x)) [> `a - x` would underflow <];
  
  if ((((int32_t)b < 0) && ((int32_t)a > (int32_t)0x7fffffff + (int32_t)b)) ||
      (((int32_t)b > 0) && ((int32_t)a < (int32_t)0x80000000 + (int32_t)b))){
    rtl_set_OF(&one);
  }else{
    rtl_set_OF(&zero);
  }

  print_asm_template2(sub);
}

make_EHelper(cmp) {
  // TODO();
  //typically used in conjunction with conditional jumps and the SETcc instruction
  
  // WARN the following is copied from sub
  // TODO as for flags, not implemented yet
  // we need to understand 
  rtlreg_t a = id_dest->val; 
  rtlreg_t b = id_src->val; 
  rtlreg_t one = 1;
  rtlreg_t zero = 0;
  if(a < b)
    rtl_set_CF(&one);
  else
    rtl_set_CF(&zero);

  // PF is useless yet
  // AF maybe useful, not now
  
  // printf("compare result is : %x %x %x\n",a, b, a - b);

  // if ((x < 0) && (a > INT_MAX + x)) [> `a - x` would overflow <];
  // if ((x > 0) && (a < INT_MIN + x)) [> `a - x` would underflow <];
  
  if ((((int32_t)b < 0) && ((int32_t)a > (int32_t)0x7fffffff + (int32_t)b)) ||
      (((int32_t)b > 0) && ((int32_t)a < (int32_t)0x80000000 + (int32_t)b))){
    rtl_set_OF(&one);
    // printf("compare underflow\n");
  }else{
    rtl_set_OF(&zero);
    // printf("compare not underflow\n");
  }

  a -= b;
  rtl_update_ZFSF(&a, decoding.dest.width);

  print_asm_template2(cmp);
}

make_EHelper(inc) {
  // TODO();
  int t2 = id_dest->val;

  id_dest->val ++;
  operand_write(id_dest, &id_dest->val);
  rtl_update_ZFSF(&id_dest->val, decoding.dest.width);

  t0 = 0;
  t1 = 1;
  if(t2 != 0xffffffff){
    rtl_set_CF(&t0);
  }else{
    rtl_set_CF(&t1);
  }

  if(t2 != 0x7fffffff){
    rtl_set_OF(&t0);
  }else{
    rtl_set_OF(&t1);
  }

  print_asm_template1(inc);
}

make_EHelper(dec) {
  t2 = id_dest->val;
  id_dest->val --;
  operand_write(id_dest, &id_dest->val);
  rtl_update_ZFSF(&id_dest->val, decoding.dest.width);

  t0 = 0;
  t1 = 1;
  if(t2 != 0){
    rtl_set_CF(&t0);
  }else{
    rtl_set_CF(&t1);
  }

  if(t2 != 0x80000000){
    rtl_set_OF(&t0);
  }else{
    rtl_set_OF(&t1);
  }
  
  print_asm_template1(dec);
}

make_EHelper(neg) {
  // TODO();

  // IF r/m = 0 THEN CF := 0 ELSE CF := 1; FI;
  // r/m := - r/m;
  rtlreg_t one = 1;
  rtlreg_t zero = 0;
  if(id_dest->val == 0){
    rtl_set_CF(&zero);
  }else{
    rtl_set_CF(&one);
  }

  if(id_dest->val == 0x80000000){
    rtl_set_OF(&one);
  }else{
    rtl_set_OF(&zero);
  }

  id_dest->val = - id_dest->val;

  rtl_update_ZFSF(&id_dest->val, id_dest->width);

  operand_write(id_dest, &id_dest->val);
  print_asm_template1(neg);
}

make_EHelper(adc) {
  // ADC performs an integer addition of the two operands DEST and SRC and the carry flag, CF.
  // The result of the addition is assigned to the first operand (DEST),
  // and the flags are set accordingly.
  // ADC is usually executed as part of a multi-byte or multi-word addition operation. 
  // When an immediate byte value is added to a word or doubleword operand,
  // the immediate value is first sign-extended to the size of the word or doubleword operand.
  
  // void operand_write(Operand *op, rtlreg_t* src);
  rtl_add(&t2, &id_dest->val, &id_src->val);
  rtl_setrelop(RELOP_LTU, &t3, &t2, &id_dest->val); // 
  rtl_get_CF(&t1);
  rtl_add(&t2, &t2, &t1);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_setrelop(RELOP_LTU, &t0, &t2, &id_dest->val);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_not(&t0, &t0);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(adc);
}

make_EHelper(sbb) {
  rtl_sub(&t2, &id_dest->val, &id_src->val);
  rtl_setrelop(RELOP_LTU, &t3, &id_dest->val, &t2);
  rtl_get_CF(&t1);
  // printf("sbb get borrow bit is %x\n", t1);
  rtl_sub(&t2, &t2, &t1);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_setrelop(RELOP_LTU, &t0, &id_dest->val, &t2);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(sbb);
}

make_EHelper(mul) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_mul_lo(&t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr(R_AX, &t1, 2);
      break;
    case 2:
      rtl_sr(R_AX, &t1, 2);
      rtl_shri(&t1, &t1, 16);
      rtl_sr(R_DX, &t1, 2);
      break;
    case 4:
      rtl_mul_hi(&t2, &id_dest->val, &t0);
      rtl_sr(R_EDX, &t2, 4);
      rtl_sr(R_EAX, &t1, 4);
      break;
    default: assert(0);
  }

  print_asm_template1(mul);
}


// imul with one operand
make_EHelper(imul1) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_imul_lo(&t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr(R_AX, &t1, 2);
      break;
    case 2:
      rtl_sr(R_AX, &t1, 2);
      rtl_shri(&t1, &t1, 16);
      rtl_sr(R_DX, &t1, 2);
      break;
    case 4:
      rtl_imul_hi(&t2, &id_dest->val, &t0);
      rtl_sr(R_EDX, &t2, 4);
      rtl_sr(R_EAX, &t1, 4);
      break;
    default: assert(0);
  }

  print_asm_template1(imul);
}

// imul with two operands
make_EHelper(imul2) {
  rtl_sext(&t0, &id_src->val, id_src->width);
  rtl_sext(&t1, &id_dest->val, id_dest->width);

  // printf("imul2 get value %x %x %x %x\n", id_dest->val, id_src->val, id_dest->width, id_src->width);
  // printf("imul2 ext value %x %x\n", t0, t1);

  rtl_imul_lo(&t2, &t1, &t0);
  operand_write(id_dest, &t2);

  print_asm_template2(imul);
}

// imul with three operands
make_EHelper(imul3) {
  rtl_sext(&t0, &id_src->val, id_src->width);
  rtl_sext(&t1, &id_src2->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul_lo(&t2, &t1, &t0);
  operand_write(id_dest, &t2);

  print_asm_template3(imul);
}

make_EHelper(div) {
  switch (id_dest->width) {
    case 1:
      rtl_lr(&t0, R_AX, 2);
      rtl_div_q(&t2, &t0, &id_dest->val);
      rtl_div_r(&t3, &t0, &id_dest->val);
      rtl_sr(R_AL, &t2, 1);
      rtl_sr(R_AH, &t3, 1);
      break;
    case 2:
      rtl_lr(&t0, R_AX, 2);
      rtl_lr(&t1, R_DX, 2);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_div_q(&t2, &t0, &id_dest->val);
      rtl_div_r(&t3, &t0, &id_dest->val);
      rtl_sr(R_AX, &t2, 2);
      rtl_sr(R_DX, &t3, 2);
      break;
    case 4:
      rtl_lr(&t0, R_EAX, 4);
      rtl_lr(&t1, R_EDX, 4);
      rtl_div64_q(&cpu.eax, &t1, &t0, &id_dest->val);
      rtl_div64_r(&cpu.edx, &t1, &t0, &id_dest->val);
      break;
    default: assert(0);
  }

  print_asm_template1(div);
}

make_EHelper(idiv) {
  switch (id_dest->width) {
    case 1:
      rtl_lr(&t0, R_AX, 2);
      rtl_idiv_q(&t2, &t0, &id_dest->val);
      rtl_idiv_r(&t3, &t0, &id_dest->val);
      rtl_sr(R_AL, &t2, 1);
      rtl_sr(R_AH, &t3, 1);
      break;
    case 2:
      rtl_lr(&t0, R_AX, 2);
      rtl_lr(&t1, R_DX, 2);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_idiv_q(&t2, &t0, &id_dest->val);
      rtl_idiv_r(&t3, &t0, &id_dest->val);
      rtl_sr(R_AX, &t2, 2);
      rtl_sr(R_DX, &t3, 2);
      break;
    case 4:
      rtl_lr(&t0, R_EAX, 4);
      rtl_lr(&t1, R_EDX, 4);
      rtl_idiv64_q(&cpu.eax, &t1, &t0, &id_dest->val);
      rtl_idiv64_r(&cpu.edx, &t1, &t0, &id_dest->val);
      break;
    default: assert(0);
  }

  print_asm_template1(idiv);
}
