#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t res = 0;
  while (s[res] != '\0')
    res++;
  return res;
}

char *strcpy(char *dest, const char *src) {
  size_t i;
  for (i = 0; src[i] != '\0'; i++)
    dest[i] = src[i];
  return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
  size_t i;

  for (i = 0; i < n && src[i] != '\0'; i++)
    dest[i] = src[i];
  for (; i < n; i++)
    dest[i] = '\0';

  return dest;
}

char *strcat(char *dest, const char *src) {
  size_t dest_len = strlen(dest);
  size_t i;

  for (i = 0; src[i] != '\0'; i++)
    dest[dest_len + i] = src[i];
  dest[dest_len + i] = '\0';

  return dest;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while (n && *s1 && (*s1 == *s2)) {
    s1++;
    s2++;
    n--;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void *memset(void *b, int c, size_t len) {
  unsigned char *p = b;
  while (len > 0) {
    *p = c;
    p++;
    len--;
  }
  return (b);
}

void *memcpy(void *out, const void *in, size_t len) {
  unsigned char *pout = out;
  const unsigned char *pin = in;
  while (len > 0) {
    *pout = *pin;
    pout++;
    pin++;
    len--;
  }
  return (out);
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *ps1 = s1;
  const unsigned char *ps2 = s2;
  while (n && (*ps1 == *ps2)) {
    ps1++;
    ps2++;
    n--;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

#endif
