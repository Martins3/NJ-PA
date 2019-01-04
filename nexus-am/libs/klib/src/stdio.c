#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)



int static puts(char * out, unsigned int * pj, const char * src){
  int len = 0;
  int i = 0;
  while(src[i] != '\0'){
    out[*pj] = src[i];
    *pj = *pj + 1;
    i ++;
    len ++;
  }
  return len;
}

int static _puts(const char * str){
  int len = 0;
  while(*str != '\0'){
    _putc(*str);
    str ++;
    len ++;
  }
  return len;
}

static char * convert(unsigned int num, int base) {
  static char Representation[] = "0123456789ABCDEF";
  static char buffer[50];
  char *ptr;

  ptr = &buffer[49];
  *ptr = '\0';

  do {
    *--ptr = Representation[num % base];
    num /= base;
  } while (num != 0);

  return (ptr);
}

// WARN we should not use copy an paste
int printf(const char *fmt, ...) {
  const char *traverse;
  unsigned int i;
  char *s;
  int len = 0;
  
  // Module 1: Initializing Myprintf's arguments
  va_list arg;
  va_start(arg, fmt);

  for (traverse = fmt; *traverse != '\0'; traverse++) {
    while (*traverse != '%' && *traverse != '\0') {
      // out[j++] = *traverse;
      _putc(*traverse);
      len ++;
      traverse++;
    }

    if(*traverse == '\0'){
      // out[j++] = *traverse;
      // no need to output this !
      // no need to add
      break;
    }

    traverse++;

    // Module 2: Fetching and executing arguments
    switch (*traverse) {
    case 'c':
      i = va_arg(arg, int); // Fetch char argument
      // out[j++] = i;
      _putc(i);
      len ++;
      break;

    case 'd':
      i = va_arg(arg, int); // Fetch Decimal/Integer argument
      if (i < 0) {
        i = -i;
        // out[j++] = '-';
        _putc('-');
        len ++;
      }
      // puts(out, &j, convert(i, 10));
      len += _puts(convert(i, 10));
      break;

    case 'o':
      i = va_arg(arg, unsigned int); // Fetch Octal representation
      // puts(out, &j, convert(i, 8));
      len += _puts(convert(i, 8));
      break;

    case 's':
      s = va_arg(arg, char *); // Fetch string
      // puts(out, &j, s);
      len += _puts(s);
      break;

    case 'x':
      i = va_arg(arg, unsigned int); // Fetch Hexadecimal representation
      // puts(out, &j, convert(i, 16));
      len += _puts(convert(i, 16));
      break;
    }
  }

  // Module 3: Closing argument list to necessary clean-up
  va_end(arg);

  return len;
}

int sprintf(char *out, const char *fmt, ...) {
  const char *traverse;
  unsigned int i;
  char *s;
  unsigned int j = 0;

  int len = 0;

  // Module 1: Initializing Myprintf's arguments
  va_list arg;
  va_start(arg, fmt);

  for (traverse = fmt; *traverse != '\0'; traverse++) {
    while (*traverse != '%' && *traverse != '\0') {
      out[j++] = *traverse;
      traverse++;
      len ++;
    }

    if(*traverse == '\0'){
      out[j++] = *traverse;
      break;
    }

    traverse++;

    // Module 2: Fetching and executing arguments
    switch (*traverse) {
    case 'c':
      i = va_arg(arg, int); // Fetch char argument
      out[j++] = i;
      len ++;
      break;

    case 'd':
      i = va_arg(arg, int); // Fetch Decimal/Integer argument
      if (i < 0) {
        i = -i;
        out[j++] = '-';
        len ++;
      }
      len += puts(out, &j, convert(i, 10));
      break;

    case 'o':
      i = va_arg(arg, unsigned int); // Fetch Octal representation
      len += puts(out, &j, convert(i, 8));
      break;

    case 's':
      s = va_arg(arg, char *); // Fetch string
      len += puts(out, &j, s);
      break;

    case 'x':
      i = va_arg(arg, unsigned int); // Fetch Hexadecimal representation
      len += puts(out, &j, convert(i, 16));
      break;
    }
  }

  // Module 3: Closing argument list to necessary clean-up
  va_end(arg);

  return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {

  return 0;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  return 0;
}

#endif
