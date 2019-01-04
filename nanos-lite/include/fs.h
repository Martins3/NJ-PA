#ifndef __FS_H__
#define __FS_H__

#include "common.h"

#ifndef SEEK_SET
enum {SEEK_SET, SEEK_CUR, SEEK_END};
#endif

// this made native can not compile !
// but I can not figure out how to do it !
typedef size_t ssize_t;
typedef long int off_t;

int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);
ssize_t fs_filesz(int fd);
#endif
