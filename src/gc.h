#ifndef GC_H
#define GC_H

#include "fixnum.h"

void* mem_alloc(Fixnum length);
void  mem_free(void *ptr);

#endif
