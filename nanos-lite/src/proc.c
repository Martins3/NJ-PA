#include "proc.h"

#define MAX_NR_PROC 4

void context_kload(PCB *pcb, void *entry);
void context_uload(PCB *pcb, const char *filename);

static PCB pcb[MAX_NR_PROC] __attribute__((used));
static PCB pcb_boot;
PCB *current;
// static PCB fg_pcb;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite for the %dth time!", j);
    j ++;
    _yield();
  }
}

// #define PA3
// #define HELLO_FUN
// #define LAST_ONE
#define SHOW_OFF

void init_proc() {
  char * program = "/bin/init";
  context_uload(&pcb[0], program);
  switch_boot_pcb();
  return;

#ifdef PA3
  char * program = "/bin/hello";
  naive_uload(NULL, program);
  return;
#endif

  

#ifdef LAST_ONE
  context_uload(&pcb[0], "/bin/init");
  context_uload(&pcb[1], "/bin/hello");
  switch_boot_pcb();
  return;
#endif

// show
#ifdef SHOW_OFF
  context_uload(&pcb[0], "/bin/init");
  context_uload(&pcb[1], "/bin/hello");
  // context_uload(&pcb[2], "/bin/init");
  // context_uload(&pcb[3], "/bin/init");
  switch_boot_pcb();
  return;
#endif

  // context_kload(&pcb[0], (void *)hello_fun);

  context_uload(&pcb[0], "/bin/init");
  context_uload(&pcb[1], "/bin/hello");
  switch_boot_pcb();
}

// return the context of process which is going to run

int change_to_who = 1;
static int counter = 0;
_Context* schedule(_Context *prev) {

  Log("make a schedule");

  counter ++;
  // save the context pointer
  current->cp = prev;

  // always select pcb[0] as the new process
  // becuase this is the first process we wanna to use
  // maybe we call it as init process
  
  // get the current is ok
  // Log("switch to pcb 0");
  // current = &pcb[0];
  
  PCB * old = current;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  if(current == &pcb[1] && counter % 100){
    current = &pcb[0];
  }
  if(old != current){
    Log("make a process change");
  }



  // current = (current == &pcb[0] ? &pcb[change_to_who] : &pcb[0]);
  // if(current == &pcb[change_to_who] && counter % 100){
    // current = &pcb[0];
  // }

  // then return the new context
  return current->cp;
}
