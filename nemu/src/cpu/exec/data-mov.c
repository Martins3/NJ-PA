#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val); // operand_write can write to reg and memory
  print_asm_template2(mov);
}

make_EHelper(push) {
  // extract the data from the line
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&id_dest->val);
  operand_write(id_dest, &id_dest->val);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  // TODO();
  t0 = cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t0);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);
  print_asm("pusha");
}

make_EHelper(popa) {
  // TODO();
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t0);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);
  print_asm("popa");
}

make_EHelper(leave) {
  // TODO();
  // WARN find out what leave shoud do
  // WARN why there is segment fault if we do nothing !

  // mov esp, ebp
  // pop ebp
  // 10002c:	55                   	push   %ebp
  // 10002d:	89 e5                	mov    %esp,%ebp
  cpu.esp = cpu.ebp;
  rtl_pop(&cpu.ebp);
  // print_asm doesn't print anything just if instruction not exposed to cpu_warper
  // never call exec

  print_asm("leave");
}

// signed extended
make_EHelper(cltd) {
  // cwd ax -> dx:ax
  if (decoding.is_operand_size_16) {
    if(cpu.eax  & (1 << 15)){
      cpu.edx |= 0xffff;
    }else{
      cpu.edx &= 0xffff0000;
    }
  }

  // cdq eax -> edx
  else {
    if(cpu.eax  & (1 << 31)){
      cpu.edx |= 0xffffffff;
    }else{
      cpu.edx = 0;
    }
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  // cwde  ax->eax
  if (decoding.is_operand_size_16) {
    // TODO();
    if(cpu.eax  & (1 << 15)){
      cpu.eax |= 0xffff0000;
    }else{
      cpu.eax &= 0xffff;
    }
  }

  // cbw  al -> ax
  else {
    // TODO();
    if(cpu.eax  & (1 << 7)){
      cpu.eax |= 0xff00;
    }else{
      cpu.eax &= 0xff;
    }
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  // printf("movsx get source value %x  width %x\n", id_src->val, id_dest->width);
  rtl_sext(&t0, &id_src->val, id_src->width);
  operand_write(id_dest, &t0);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}


make_EHelper(lea) {
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}

make_EHelper(stos) {
  // operand_write(id_dest, &id_src->addr);
  // maybe is cpu.es << 2 + cpu.edi, I don't know yet
  id_dest->addr = cpu.es + cpu.edi;
  id_dest->type = OP_TYPE_MEM;
  operand_write(id_dest, &cpu.eax);
  // WARN We don't implement DF at all, maybe cause some probblem
  if(id_dest->width == 4){
    cpu.edi += 4;
  }
  
  else if(id_dest->width == 2){
    cpu.edi += 2;
  }
  
  else{
    cpu.edi += 1;
  }
  print_asm_template2(lea);
}

// WARN I can not understand why movs and movsb
// has same opcode but has different effect

make_EHelper(movs) {
  // operand_write(id_dest, &id_src->addr);
  id_dest->addr = cpu.es + cpu.edi;
  id_dest->type = OP_TYPE_MEM;

  id_src->addr = cpu.esi;
  rtl_lm(&id_src->val, &id_src->addr, id_src->width);

  operand_write(id_dest, &id_src->val);
  // WARN We don't implement DF at all
  if(id_dest->width == 4){
    cpu.edi += 4;
    cpu.esi += 4;
  }
  
  else if(id_dest->width == 2){
    cpu.edi += 2;
    cpu.esi += 2;
  }
  
  else{
    cpu.edi += 1;
    cpu.esi += 1;
  }
  print_asm_template2(lea);
}


make_EHelper(cmps) {
  TODO();
}
