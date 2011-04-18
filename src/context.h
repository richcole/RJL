#ifndef CONTEXT_H
#define CONTEXT_H

#include "object.h"

Object* new_context();
Object* context_get(Object *cxt, char const* slot);
void context_set(Object *cxt, char const* slot, Object *val);
Object* get_symbol_table(Object *cxt);

#endif
