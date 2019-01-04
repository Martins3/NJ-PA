#include "common.h"
#include "fs.h"
#include "proc.h"
#include "syscall.h"

// do we have better way to handle it !
// int fs_open(const char *pathname, int flags, int mode);
// ssize_t fs_read(int fd, void *buf, size_t len);
// ssize_t fs_write(int fd, const void *buf, size_t len);
// off_t fs_lseek(int fd, off_t offset, int whence);
// int fs_close(int fd);

// doing syscall may change the context
int mm_brk(uintptr_t new_brk);
void context_uload(PCB *pcb, const char *filename);
void naive_uload(PCB *pcb, const char *filename);

_Context *do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; // syscall number
  a[1] = c->GPR2; // fd
  a[2] = c->GPR3; // buf
  a[3] = c->GPR4; // count
  // printf("get syscall number is %x\n", a[0]);

  switch (a[0]) {
  case SYS_yield:
    _yield();
    c->GPRx = 0;
    break;
  case SYS_exit:
    _halt(a[1]);
    break;
  case SYS_brk:
    // Log function cause problem, unbelieveable
    Log("sys_brk");
    mm_brk(a[1]);
    c->GPRx = 0;
    break;
  case SYS_open:
    c->GPRx = fs_open((char *)a[1], 0, 0);
    break;
  case SYS_write:
    c->GPRx = fs_write(a[1], (const void *)a[2], a[3]);
    break;
  case SYS_close:
    c->GPRx = 0;
    break;
  case SYS_read:
    c->GPRx = fs_read(a[1], (void *)a[2], a[3]);
    break;
  case SYS_lseek:
    c->GPRx = fs_lseek(a[1], a[2], a[3]);
    break;
  case SYS_execve:
    // panic("error, no such thing as this");
    // panic("naive nload is used for what");
    // naive_uload(NULL, (char *)a[1]);
    naive_uload(current, (char *)a[1]);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }

  return c;
}
