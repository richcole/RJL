#ifndef GC_H
#define GC_H

void* mem_alloc(Fixnum length);
void  mem_free(void *ptr);

#endif
