#include <am.h>
#include <x86.h>
#include <amdev.h>

size_t input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _KbdReg *kbd = (_KbdReg *)buf;
      int32_t scancode = inl(0x60);
      kbd->keydown = scancode & (1 << 31);
      kbd->keycode = scancode & ~(1 << 31);
      return sizeof(_KbdReg);
    }
  }
  return 0;
}
