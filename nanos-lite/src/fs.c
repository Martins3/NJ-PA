#include "fs.h"
#include <string.h>

size_t serial_write(const void *buf, size_t offset, size_t len);

size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  off_t open_offset; // 文件被打开之后的读写指针
  ReadFn read;
  WriteFn write;
} Finfo;

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB };

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    {"stdin", 0, 0, 0, invalid_read, invalid_write},
    {"stdout", 0, 0, 0, invalid_read, serial_write},
    {"stderr", 0, 0, 0, invalid_read, serial_write},
    {"/dev/tty", 0, 0, 0, invalid_read, serial_write},
    {"/proc/dispinfo", 128, 0, 0, dispinfo_read, invalid_write},
#include "files.h"
    {"/dev/fb", 0, 0, 0, invalid_read, invalid_write},
    {"/dev/events", 1, 0, 0, invalid_read, invalid_write},
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
  // should we get dynamic 
  // Log("we should init the /dev/fp, but not now !");
  for (int i = 0; i < NR_FILES; ++i) {
    if (strcmp(file_table[i].name, "/dev/fb") == 0) {
      file_table[i].size = screen_width() * screen_height() * 4;
      Log("set /dev/fp size");
      return;
    }
  }
}

int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < NR_FILES; ++i) {
    if (strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("[%s] This file doesn't exit !\n", pathname);
  return 1;
}

size_t events_read(void *buf, size_t offset, size_t len);
ssize_t fs_read(int fd, void *buf, size_t len) {
  // _yield();
  if (file_table[fd].open_offset + len > file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].open_offset;
    if (len == 0){
      Log("Read error %d", fd);
      return 0;
    }
  }
  ssize_t r ;
  // WARN
  if(fd == 4){
    r = (file_table[fd]).read(buf,file_table[fd].open_offset,len);
  }
  
  else if(fd == NR_FILES - 1){
    return events_read(buf, 0, len);
  }
  
  else{
    r = ramdisk_read(
      buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  }
  file_table[fd].open_offset += len;
  return r;
}

size_t fb_write(const void *buf, size_t offset, size_t len);

ssize_t fs_write(int fd, const void *buf, size_t len) {
  // Never shoule we let the open_offset > len
  // because there are lines for the case
  if(fd <= 3){
    // printf("stdio and stdin test\n");
    return (file_table[fd]).write(buf,0,len);
  }

  if (file_table[fd].open_offset + len > file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].open_offset;
    if (len == 0)
      return 0;
  }

  ssize_t r;
  if(fd == NR_FILES - 2){
    r = fb_write(buf, file_table[fd].open_offset, len);
  }else{
    r = ramdisk_write(
        buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  }

  file_table[fd].open_offset += len;
  return r;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
  off_t *o = &(file_table[fd].open_offset);
  if (offset < 0 || offset > file_table[fd].size) {
    return -1;
  }
  switch (whence) {
  case SEEK_SET:
    *o = offset;
    break;
  case SEEK_CUR:
    if (*o + offset > file_table[fd].size) {
      return 0;
    }
    *o += offset;
    // printf("return the seek_cur %d\n", *o);
    break;
  case SEEK_END:
    // file_table[fd].open_offset = file_table[fd].size + offset;
    *o = file_table[fd].size;
    // printf("to the end %d\n", *o);
    break;
  default:
    panic("Can't reach here !\n");
  }
  return *o;
}

ssize_t fs_filesz(int fd) { return file_table[fd].size; }

int fs_close(int fd) { return 0; }
