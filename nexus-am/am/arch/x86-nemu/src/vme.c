#include <x86.h>
#include <stdio.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/\------ OFF(va) ------/

// typedef uint32_t PTE;
// typedef uint32_t PDE;
// #define NR_PTE    1024    // # PTEs per page table

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*pgalloc_usr)(size_t);
static void (*pgfree_usr)(void*);


_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))


int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
 pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  int i;

  // Make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    // now that we only have one kernel mapping
    // why we have multiple kernel mappings

    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);

    // kpdirs has a bigger table than needed
    // not all the area are mapped
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      // why there is PTE_P and what it used for.
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      // construct virtual address from indexes and offset
      // #define PGADDR(d, t, o) ((uint32_t)((d) << PDXSHFT | (t) << PTXSHFT | (o)))
      // #define PTE_P     0x001   // Present
      // #define PGSHFT    12      // log2(PGSIZE)
      // #define PTXSHFT   12      // Offset of PTX in a linear address
      
      // What should be stored into page table entry
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;


      // printf("%x %x %x %x\n", PGSIZE, pte_end, pte, pdir_idx);
      // make a for loop and exec 1 << 12 times
      for (; pte < pte_end; pte += PGSIZE) {
        // printf("[%x  %x]", pte, pdir_idx);
        *ptab = pte;
        ptab ++;
      }
      // printf("\n");
    }

  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);

  return 0;
}

// pgsize用于指示页面的大小,
// area表示虚拟地址空间中用户态的范围
// ptr则用于指示具体的映射.
// 在PA中, 目前只会用到ptr.
// make new a address space
// create a default address space
int _protect(_Protect *p) {
  // it seems that it just want to get some pages
  // and read some lines, and find a something new
  // get one page is enough to create the directory
  PDE *updir = (PDE*)(pgalloc_usr(1));
  p->pgsize = 4096;
  p->ptr = updir;

  // map kernel space
  // what does this mean, copy the whole kpdir
  // when malloc the updir
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  // now that area is unused, why should we change this ?
  // how compile know what is happening, maybe it's used for
  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
  return 0;
}

void _unprotect(_Protect *p) {
}


static _Protect *cur_as = NULL;
void get_cur_as(_Context *c) {
  c->prot = cur_as;
}

// who will call this
void _switch(_Context *c) {
  set_cr3(c->prot->ptr);
  cur_as = c->prot;
}

// doing the map, how and why ?
// mode, what does this used for ?
// becase p copy all the context, but it can't
// now that you copy the dir

// a doubt has long last in my view
// why they can seperate each other address
// they copy the same value and set address the same way
// provide va and pa 

// we have change, maybe we don't have the table
// make for loop, and use the present bit to alloc

// 若在映射过程中发现需要申请新的页表, 可以通过回调函数pgalloc_usr()向Nanos-lite获取一页空闲的物理页.

// kernel map is what we can not modify, we have 
// apprently, we page and page table are used from the same place
// what if this problem can makes a line
int _map(_Protect *p, void *va, void *pa, int mode) {
  // printf("make a map from va[%x] tp pa[%x]\n", (int)va, (int)pa);

  PDE * updir = p->ptr; // index of page dir
  PTE pax = updir[PDX(va)]; // address of page table

  PTE * ptaddr;
  if(!(pax & 1)){
    ptaddr = (PTE*)(pgalloc_usr(1));
    updir[PDX(va)] = (int)ptaddr | PTE_P;
  }else{
    pax = (pax >> 1) << 1;
    ptaddr = (PTE *)pax;
  }

  ptaddr += PTX(va); // find the entry

  *ptaddr = PTE_ADDR(pa) | PTE_P;

  // mode is difficult, how to use it !
  
  return 0;
}

_Context *_ucontext(_Protect *p, _Area ustack, _Area kstack, void *entry, void *args) {
  ustack.end -= 1 * sizeof(uintptr_t);  // 1 = retaddr
  uintptr_t ret = (uintptr_t)ustack.end;
  *(uintptr_t *)ret = 0;

  _Context *c = (_Context*)ustack.end - 1;
  c->eip = (uintptr_t)entry;
  c->prot = p;
  c->eflags = 0xffffffff; // open interupt

  // maybe useful, but I can not understand this now
  // c->uc.uc_mcontext.gregs[REG_RDI] = 0;
  // c->uc.uc_mcontext.gregs[REG_RSI] = ret; // ???
  // c->uc.uc_mcontext.gregs[REG_RDX] = ret; // ???

  return c;
}
