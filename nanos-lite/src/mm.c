#include "memory.h"
#include <stdio.h>
#include "proc.h"

static void *pf = NULL;

// malloc some pages
// things becomes complex if there are multiple address space
// maloc from zero and return can make a line
void* new_page(size_t nr_page) {
  void *p = pf;
  pf += PGSIZE * nr_page;
  assert(pf < (void *)_heap.end);
  // printf("alloc a page %x\n", (int)p);
  return p;
}

void free_page(void *p) {
  panic("not implement yet, and we don't need this in PA");
}

// all function start at the same place, something is 
// understand clearly.
/* The brk() system call handler. */
// extern char _end;
// uintptr_t max_brk = (uintptr_t)&_end;

int mm_brk(uintptr_t new_brk) {
  // if(new_brk > max_brk){
  if(current->max_brk > new_brk){
    printf("max_brk %x is bigger than new_brk %x\n", current->max_brk, new_brk);
    return 0;
  }

  for (; current->max_brk < new_brk; current->max_brk += PGSIZE) {
    void * pa = new_page(1);
    _map(&(current->as), (void *)(current->max_brk), pa, 0);
  }
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("pf start at 0x%x", pf);

  _vme_init(new_page, free_page);
  Log("init vm over");
}

// TRM init the heap start, where is it ?
// sequential way, why we have this ?

// 目前初始化MM的工作有两项,
// 第一项工作是将TRM提供的堆区起始地址作为空闲物理页的首地址,
// 将来会通过new_page()函数来分配空闲的物理页. 为了简化实现,
// MM中采用顺序的方式对物理页进行分配, 而且分配后无需回收.
// 第二项工作是调用AM的_vme_init()函数, 填写内核的页目录和页表,
// 然后设置CR3寄存器, 最后通过设置CR0寄存器来开启分页机制.
// 这样以后, Nanos-lite就运行在分页机制之上了.
