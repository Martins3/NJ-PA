#include "common.h"
#include <stdio.h>

_Context* schedule(_Context *prev); // inlcude from proc

_Context* do_syscall(_Context *c);
static _Context* do_event(_Event e, _Context* c) {
  switch (e.event) {
    case _EVENT_YIELD:
      // Log("switch process");
      return schedule(c);
      break;
    case _EVENT_SYSCALL:
      return do_syscall(c);
      break;
    case _EVENT_IRQ_TIMER:
      // Log("timer log");
      return schedule(c);
      // _yield();
      break;
    default:
      panic("Unhandled event ID = %d", e.event);
  }
  // i don't know why should return context again !
  // return NULL
  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
