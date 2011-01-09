
typedef Object* (*FuncPtr)(Object *parent);

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
  if ( object != 0 && object->buffer != 0 && object->buffer->type == Func ) {
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

Object *call_func(Object *frame, Object *func) {
  FuncBuffer *func_buf = get_func_buffer(func);
  if ( func_buf != 0 ) {
    return (*func_buf->func)(frame);
  }
  return new_exception(frame, "Expected a function object");
}

Object *native_call(Object *self, Object *slot) {
	Object *frame = new_frame();
	Object *stack = get(frame, Stack);
	Object *func  = get(self, slot);
	frame = call_func(frame, func);
	return pop(stack);
} 

Object *native_call(Object *self, Object *slot, Object *arg) {
	Object *frame = new_frame();
	Object *stack = get(frame, Stack);
	push(stack, arg);
	Object *func = get(self, slot);
	frame = call_func(frame, func);
	return pop(stack);
} 

void init_func_symbols() {
	add_sym(Func, "Func");
}

