#include "device/mmio.h"
#include "nemu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type)                                                    \
  *(type *)({                                                                  \
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound",       \
           addr);                                                              \
    guest_to_host(addr);                                                       \
  })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

// 在NEMU中的paddr_read()和paddr_write()中加入对内存映射I/O的判断.
// 通过is_mmio()函数判断一个物理地址是否被映射到I/O空间,
// 如果是, is_mmio()会返回映射号, 否则返回-1.
// 内存映射I/O的访问需要调用mmio_read()或mmio_write(),
// 调用时需要提供映射号. 如果不是内存映射I/O的访问, 就访问pmem.

uint32_t paddr_read(paddr_t addr, int len) {
  int map_NO = is_mmio(addr);
  if (map_NO != -1) {
    return mmio_read(addr, len, map_NO);
  }

  else {
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
  int map_NO = is_mmio(addr);
  if (map_NO != -1) {
    mmio_write(addr, len, data, map_NO);
  }

  else {
    memcpy(guest_to_host(addr), &data, len);
  }
}

static int is_data_cross_the_page_boundry(vaddr_t addr, int len) {
  addr = ((uint32_t)(addr)&0xfff);
  if (addr + len > (1 << 12))
    return addr + len - (1 << 12);
  return 0;
}


#define PDX(va) (((uint32_t)(va) >> 22) & 0x3ff)
#define PTX(va) (((uint32_t)(va) >> 12) & 0x3ff)
#define OFF(va) ((uint32_t)(va)&0xfff)

// what is the relation with the map ?
paddr_t page_translate(vaddr_t va) {
  if (cpu.cr0.paging) {
    paddr_t page_dir = cpu.cr3.val;
    paddr_t dir_entry_addr = page_dir + (PDX(va) << 2);
    PDE page_tab =(PDE)paddr_read(dir_entry_addr, 4);

    if(!page_tab.present){
      printf("Maybe we are break something down at page dir\n");
      assert(0);
    }


    paddr_t tab_entry_addr = (page_tab.val & (~1)) + (PTX(va) << 2);
    PTE tab_entry = (PTE)paddr_read(tab_entry_addr, 4);
    if(!tab_entry.present){
      printf("Maybe we are break something down at page table %x\n", va);
      assert(0);
    }

    return OFF(va) | (tab_entry.page_frame << 12);
  }

  else {
    // printf("I am in the real mode, just return the address is enough\n");
    return va;
  }
}

// maybe we should use get_cr0 ?
// but I can not understand the function of cr0
uint32_t vaddr_read(vaddr_t addr, int len) {
  int overflow = is_data_cross_the_page_boundry(addr, len);
  if (overflow) {
    /* this is a special case, you can handle it later. */
    paddr_t low_paddr = page_translate(addr);
    int low = paddr_read(low_paddr, len - overflow);
    paddr_t high_paddr = page_translate(addr + len - overflow);
    int high = paddr_read(high_paddr, overflow);
    return high <<(8 *(len - overflow)) | low;
  } else {
    paddr_t paddr = page_translate(addr);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, uint32_t data, int len) {
  int overflow = is_data_cross_the_page_boundry(addr, len);
  if (overflow) {
    /* this is a special case, you can handle it later. */
    // assert(0);
    paddr_t low_paddr = page_translate(addr);
    paddr_write(low_paddr, data, len - overflow);
    paddr_t high_paddr = page_translate(addr + len - overflow);
    paddr_write(high_paddr, data >> (8 * (len - overflow)), overflow);
  } else {
    paddr_t paddr = page_translate(addr);
    paddr_write(paddr, data, len);
  }
}
