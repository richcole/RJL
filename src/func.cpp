#include "func.h"
#include "mem.h"
#include "sym.h"
#include "exception.h"
#include "frame.h"
#include "array.h"
#include "type_tags.h"

FuncBuffer* new_func_buffer(Object *cxt, FuncPtr func_ptr) {
  FuncBuffer *buf = (FuncBuffer *)mem_alloc(sizeof(FuncBuffer));
  buf->type = FuncTypeTag;
  buf->func = func_ptr;
  return buf;
};

Fixnum is_func(Object *cxt, Object *object) {
  return get_func_buffer(object) != 0;
};

Object *new_func(Object *cxt, FuncPtr func_ptr) {
  Object *object = new_object(cxt, "Func");
  object->buffer = (Buffer *) new_func_buffer(cxt, func_ptr);
  return object;
}

def_get_buffer(Func, func, FuncTypeTag);
def_set_buffer(Func, func, FuncTypeTag);

Object *call_func(Object *cxt, Object *target, Object *func, Object *frame) {
  FuncBuffer *func_buf = get_func_buffer(func);
  if ( func_buf != 0 ) {
    return (*func_buf->func)(cxt, frame, target);
  }
  return new_exception(cxt, frame, "Expected a function object");
}

Object *native_call(Object *cxt, Object *self, Object *slot) {
  Object *frame = new_frame(cxt);
  Object *stack = get(cxt, frame, "stack");
  Object *func  = get(cxt, self, slot);
  frame = call_func(cxt, self, func, frame);
  return pop(cxt, stack);
} 

Object *native_call(Object *cxt, Object *self, char const* s) {
  return native_call(cxt, self, sym(cxt, s));
}

Object *native_call(Object *cxt, Object *self, Object *slot, Object *arg) {
	Object *frame = new_frame(cxt);
	Object *stack = get(cxt, frame, "stack");
	push(cxt, stack, arg);
	Object *func = get(cxt, self, slot);
	frame = call_func(cxt, self, func, frame);
	return pop(cxt, stack);
} 

Object *native_call(Object *cxt, Object *self, char const* s, Object *arg) {
  return native_call(cxt, self, sym(cxt, s), arg);
} 

Object *native_call(Object *cxt, Object *self, char const* s, char const* a) {
  return native_call(cxt, self, sym(cxt, s), sym(cxt, a));
} 

Object *native_call(Object *cxt, Object *self, Object *slot, Object *arg1, 
  Object *arg2) 
{
  Object *frame = new_frame(cxt);
  Object *stack = get(cxt, frame, "stack");
  push(cxt, stack, arg1);
  push(cxt, stack, arg2);
  Object *func = get(cxt, self, slot);
  frame = call_func(cxt, self, func, frame);
  return pop(cxt, stack);
} 

Object *native_call(Object *cxt, Object *self, Object *slot, Object *arg1, 
                    Object *arg2, Object *arg3) 
{
  Object *frame = new_frame(cxt);
  Object *stack = get(cxt, frame, "stack");
  push(cxt, stack, arg1);
  push(cxt, stack, arg2);
  push(cxt, stack, arg3);
  Object *func = get(cxt, self, slot);
  frame = call_func(cxt, self, func, frame);
  return pop(cxt, stack);
} 

Object *native_call(Object *cxt, Object *self, char const* slot, Object *arg1, 
                    Object *arg2, Object *arg3) 
{
  return native_call(cxt, self, sym(cxt, slot), arg1, arg2, arg3);
};
