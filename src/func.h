
typedef Object* (*FuncPtr)(Object *frame, Object *target);

struct FuncBuffer {
  Object *type;
  FuncPtr func;
};

Object *Func = new_object();

FuncBuffer* new_func_buffer(FuncPtr func_ptr) {
  FuncBuffer *buf = (FuncBuffer *)mem_alloc(sizeof(FuncBuffer));
  buf->type = Func;
  buf->func = func_ptr;
  return buf;
}

Fixnum is_func(Object *object) {
  if ( object != 0 && is_object(object) && object->buffer != 0 && object->buffer->type == Func ) {
    return 1;
  }
  return 0;
}

Object *new_func(FuncPtr func_ptr) {
  Object *object = new_object();
  object->buffer = (Buffer *) new_func_buffer(func_ptr);
  return object;
}

def_get_buffer(Func, func);

Object *call_func(Object *target, Object *func, Object *frame) {
  FuncBuffer *func_buf = get_func_buffer(func);
  if ( func_buf != 0 ) {
    return (*func_buf->func)(frame, target);
  }
  return new_exception(frame, "Expected a function object");
}

Object *native_call(Object *self, Object *slot) {
	Object *frame = new_frame();
	Object *stack = get(frame, Stack);
	Object *func  = get(self, slot);
	frame = call_func(self, func, frame);
	return pop(stack);
} 

Object *native_call(Object *self, char const* s) {
  return native_call(self, sym(s));
} 

Object *native_call(Object *self, Object *slot, Object *arg) {
	Object *frame = new_frame();
	Object *stack = get(frame, Stack);
	push(stack, arg);
	Object *func = get(self, slot);
	frame = call_func(self, func, frame);
	return pop(stack);
} 

Object *native_call(Object *self, char const* s, Object *arg) {
  return native_call(self, sym(s), arg);
} 

Object *native_call(Object *self, char const* s, char const* a) {
  return native_call(self, sym(s), sym(a));
} 

Object *native_call(Object *self, Object *slot, Object *arg1, Object *arg2) {
	Object *frame = new_frame();
	Object *stack = get(frame, Stack);
	push(stack, arg1);
	push(stack, arg2);
	Object *func = get(self, slot);
	frame = call_func(self, func, frame);
	return pop(stack);
} 

Object *native_call(Object *self, Object *slot, Object *arg1, Object *arg2, Object *arg3) {
	Object *frame = new_frame();
	Object *stack = get(frame, Stack);
	push(stack, arg1);
	push(stack, arg2);
	push(stack, arg3);
	Object *func = get(self, slot);
	frame = call_func(self, func, frame);
	return pop(stack);
} 

Object *native_call(Object *self, char const* s, Object *arg1, Object *arg2, Object *arg3) {
  return native_call(self, sym(s), arg1, arg2, arg3);
}

void init_func_symbols() {
	add_sym(Func, "Func");
}

