#ifndef CONTEXT_H
#define CONTEXT_H

#include "object.h"

Object* new_context();
Object* context_get(Object *cxt, char const* slot);
void context_set(Object *cxt, char const* slot, Object *val);
Object* get_symbol_table(Object *cxt);

void* context_alloc_buffer(Object *cxt, Fixnum length);
void context_free_buffer(Object *cxt, void *buffer);
void context_register_object(Object *cxt, Object *obj);
void context_unregister_object(Object *cxt, Object *obj);

#endif
