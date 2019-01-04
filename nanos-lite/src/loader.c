#include "proc.h"
#include "common.h"
#include "fs.h"
#include "memory.h"

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))
#define DEFAULT_ENTRY 0x4000000

static uintptr_t loader(PCB *pcb, const char *filename) {
  // read the whole ramdisk to DEFAULT_ENTRY
  // ramdisk_read((void *)DEFAULT_ENTRY, 0, RAMDISK_SIZE);
  // return DEFAULT_ENTRY;
  
  // read specific ramdisk into DEFAULT_ENTRY
  // int fd = fs_open(filename, 0, 0);
  // size_t len = fs_filesz(fd);
  // fs_read(fd, (void *)DEFAULT_ENTRY, len);
  // return DEFAULT_ENTRY;

  Log("load the image into memroy");
  int fd = fs_open(filename, 0, 0);
  size_t len = fs_filesz(fd);
  // what's the meaning of mode
  // we forget the copy process
  void * va =(void *)0x8048000;
  for (;va < (void *)0x8048000 + len; va += PGSIZE) {
    void * pa = new_page(1);
     fs_read(fd, pa, PGSIZE);
    _map(&(pcb->as), va, pa, 0);
  }
  pcb->max_brk = (uintptr_t)va;
  pcb->cur_brk = (uintptr_t)va;

  return 0x8048000;
}

void naive_uload(PCB *pcb, const char *filename) {
  // panic("naive_uload is illegal here");
  uintptr_t entry = loader(pcb, filename);
  ((void(*)())entry) ();
}

/*
understand this function

typedef struct _Area {
  void *start, *end;
} _Area;

create a empty Context at the specific process stack end and set the entry
by change the rip

*/

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, NULL);
}

int _protect(_Protect *p);
void context_uload(PCB *pcb, const char *filename) {
  Log("create address space for %s", filename);
  _protect(&(pcb->as)); // create a address space
  uintptr_t entry = loader(pcb, filename);
  Log("loader finished and get entry %x", entry);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
