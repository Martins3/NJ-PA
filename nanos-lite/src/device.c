#include "common.h"
#include <amdev.h>
#include <string.h>
#include <stdio.h>

size_t serial_write(const void *buf, size_t offset, size_t len) {
  // _yield();
  char *str = (char *)buf;
  for (int i = 0; i < len; ++i) {
    // res += _syscall_(SYS_write, fd, str[i], 0);
    _putc(str[i]);
  }
  return len;
}


#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

extern int change_to_who;
size_t events_read(void *buf, size_t offset, size_t len) {
  // _yield();
  int key = read_key();
  char du[3];
  if(key  != _KEY_NONE){
    Log("key events");
    // Log("read events from vfs");
    if(key & 0x8000){
      du[0] = 'k'; du[1] = 'd'; du[2] = '\0';
    }else{
      du[0] = 'k'; du[1] = 'u'; du[2] = '\0';
    }
    key &= 0x3fff;

    if(key == 2){
      change_to_who = 1;
    }else if(key == 3){
      change_to_who = 2;
    }else if(key == 4){
      change_to_who = 3;
    }
    
    return sprintf(buf, "%s %s\n", du, keyname[key]);
  }else{
    return sprintf(buf, "t %d\n", uptime());
  }
  return 0;
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  memcpy(buf, (void *)dispinfo + offset, len);
  return len;
}

// Critical warning !!!
size_t fb_write(const void *buf, size_t offset, size_t len) { 
  //_yield();
  int w = screen_width();

  offset = offset / 4;
  int x = offset % w;
  int y = offset / w;

  // Log("%d %d\n", x, y);

  int width = len / 4;
  if(width > w) width = w;

  // it works well if the every time write one line into it.
  draw_rect((uint32_t *)buf, x,  y, width, 1);
 
  return len;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d", screen_width(), screen_height());
}
