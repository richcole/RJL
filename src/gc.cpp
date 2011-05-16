#include "gc.h"
#include "mem.h"
#include <stdlib.h>


void* mem_alloc(Fixnum length) {
  void *ptr = malloc(length);
  rjl_memset((char *)ptr, 0, length);
  return ptr;
};

void mem_free(void *ptr) {
  free(ptr);
}

