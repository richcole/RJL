#include "mem.h"

void rjl_memset(void *ptr, char val, Fixnum length) {
  char *p = (char *)ptr;
  char *q = p + length;
  while(p != q) {
    *p++ = val;
  }
}

Fixnum rjl_strlen(char const* s) {
  Fixnum i = 0;
  while(s[i] != 0) ++i;
  return i;
}

void rjl_memcpy(void *dst, void const* src, int len) {
  char *p = (char *)dst;
  char *q = (char *)src;
  while(len-- > 0) {
    *p++ = *q++;
  }
}

void   rjl_memset(char *dst, char val, int len) {
  while(len > 0) {
    *dst = val;
    --len;
    ++dst;
  }
}
