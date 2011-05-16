#ifndef MEM_H
#define MEM_H

#include "fixnum.h"

void  memset(void *ptr, char val, Fixnum length);

void   rjl_memcpy(void *dst, void const* src, int len);
Fixnum rjl_strlen(char const* s);
void   rjl_memset(char *dst, char val, int len);

#endif

