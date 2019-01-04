#include "cpu/rtl.h"

/* Condition Code */

void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  bool invert = subcode & 0x1; // what is the meaning of invert

  // names is related to the instruction
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  };

  // TODO: Query EFLAGS to determine whether the condition code is satisfied.
  // dest <- ( cc is satisfied ? 1 : 0)
  
  rtlreg_t a;
  rtlreg_t b;
  rtlreg_t c;
  switch (subcode & 0xe) {
    case CC_O:
    case CC_NO:
      rtl_get_OF(dest);
      break;


    case CC_NB:
    case CC_B:
      rtl_get_CF(dest);
      break;

    case CC_E:
    case CC_NE:
      rtl_get_ZF(dest);
      break;

    case CC_BE:
    case CC_NBE:
      rtl_get_CF(&a);
      rtl_get_ZF(&b);
      *dest = 0;
      if(a == 1 || b == 1){
        *dest = 1;
      }
      break;

    case CC_S:
    case CC_NS:
      rtl_get_SF(dest);
      break;

    case CC_NL:
    case CC_L:
      // printf("CC code hit CC_NL or CC_L\n");
      rtl_get_SF(&a);
      rtl_get_OF(&b);
      // printf("%x %x \n",a, b);
      *dest = 0;
      if(a != b){
        *dest = 1;
      }
      break;
    case CC_LE:
    case CC_NLE:
      // ((ZF=1) OR (SF!=OF))
      rtl_get_ZF(&a);
      rtl_get_SF(&b);
      rtl_get_OF(&c);
      *dest = 0;
      if(a == 1 || (b != c)){
        *dest = 1;
      }
      
      break;
      // TODO();
    default: panic("should not reach here");
    case CC_P: panic("n86 does not have PF");
    case CC_NP: panic("n86 does not have PF");
  }

  if (invert) {
    // printf("cc code cause inverted\n");
    rtl_xori(dest, dest, 0x1);
  }
  // printf("rtl_setcc result is %x\n", *dest);
}
