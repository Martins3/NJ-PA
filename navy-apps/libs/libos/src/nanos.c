#include "syscall.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#if defined(__ISA_X86__)
intptr_t _syscall_(int type, intptr_t a0, intptr_t a1, intptr_t a2) {
  int ret = -1;
  asm volatile("int $0x80" : "=a"(ret) : "a"(type), "b"(a0), "c"(a1), "d"(a2));
  return ret;
}
#elif defined(__ISA_AM_NATIVE__)
intptr_t _syscall_(int type, intptr_t a0, intptr_t a1, intptr_t a2) {
  intptr_t ret = 0;
  asm volatile("call *0x100000"
               : "=a"(ret)
               : "a"(type), "S"(a0), "d"(a1), "c"(a2));
  return ret;
}
#else
#error _syscall_ is not implemented
#endif

void _exit(int status) {
  _syscall_(SYS_exit, status, 0, 0);
  while (1)
    ;
}

int _open(const char *path, int flags, mode_t mode) {
  return _syscall_(SYS_open, (intptr_t)path, 0, 0);
}

int _write(int fd, void *buf, size_t count) {
  // stdin and stderr is file too !
  // if(fd < 3){
  // int res = 0;
  // char *str = (char *)buf;
  // for (int i = 0; i < count; ++i) {
  // res += _syscall_(SYS_write, fd, str[i], 0);
  // }
  // return res;
  // }else{
  return _syscall_(SYS_write, fd, (intptr_t)buf, count);
  // }
}



  extern char _end;
  char *pbrk = &_end;
void *_sbrk(intptr_t increment) {
  
  // void * r = (void *)(current->cur_brk);
  // _syscall_(SYS_brk, r, 0, 0);
  // current->cur_brk += increment;
  // return r;
  

  // _syscall_(SYS_brk, increment, 0, 0);
  // can not understand it now, just change it here
  char *r = pbrk;
  pbrk += increment;
  // printf("origin is %lx, increment is %x\n", (long int)r, (int)increment);
  _syscall_(SYS_brk, pbrk, 0, 0);
  return (void *)r;
}

/**
  // following code is happened to work fin

  // --> put this two line out of function
  extern char _end;
  char *pbrk = &_end;
  // --> put this part into _brk
  // _syscall_(SYS_brk, increment, 0, 0);
  // can not understand it now, just change it here
  char *r = pbrk;
  pbrk += increment;
  // printf("origin is %lx, increment is %x\n", (long int)r, (int)increment);
  _syscall_(SYS_brk, pbrk, 0, 0);
  return (void *)r;
*/

int _read(int fd, void *buf, size_t count) {
  int len = _syscall_(SYS_read, fd, (intptr_t)buf, count);
  // printf("file io %x [%s] with size %d\n", fd, (char *) buf, (int)count);
  return len;
}

int _close(int fd) { return _syscall_(SYS_close, fd, 0, 0); }

off_t _lseek(int fd, off_t offset, int whence) {
  return _syscall_(SYS_lseek, fd, offset, whence);
}

int _execve(const char *fname, char *const argv[], char *const envp[]) {
  void *f = (char *)fname;
  return _syscall_(SYS_execve, f, 0, 0);
}

// The code below is not used by Nanos-lite.
// But to pass linking, they are defined as dummy functions

int _fstat(int fd, struct stat *buf) { return 0; }

int _kill(int pid, int sig) {
  _exit(-SYS_kill);
  return -1;
}

pid_t _getpid() {
  _exit(-SYS_getpid);
  return 1;
}
