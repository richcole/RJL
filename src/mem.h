#include <stdlib.h>

void memset(void *ptr, char val, Fixnum length) {
  char *p = (char *)ptr;
  char *q = p + length;
  while(p != q) {
    *p++ = val;
  }
}

void* mem_alloc(Fixnum length) {
  void *ptr = malloc(length);
  memset(ptr, 0, length);
  return ptr;
};

void mem_free(void *ptr) {
  free(ptr);
}
