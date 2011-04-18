#ifndef FUNC_H
#define FUNC_H

#include "object.h"
#include "type_tags.h"

typedef Object* (*FuncPtr)(Object *cxt, Object *frame, Object *target);

struct FuncBuffer {
  Fixnum  type;
  FuncPtr func;
};

Fixnum is_func(Object *cxt, Object *object);
Object *new_func(Object *cxt, FuncPtr func_ptr);
Object *call_func(Object *cxt, Object *target, Object *func, Object *frame);
Object *native_call(Object *cxt, Object *self, Object *slot);

decl_get_buffer(Func, func, FuncTypeTag);
decl_set_buffer(Func, func, FuncTypeTag);

#endif
