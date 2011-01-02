
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

FuncBuffer *get_func_buffer(Object *obj) {
  if ( obj != 0 && obj->buffer != 0 && obj->buffer->type == Func ) {
    return (FuncBuffer *) obj->buffer;
  }
  return 0;
}

Object *call_func(Object *frame, Object *func) {
  FuncBuffer *func_buf = get_func_buffer(func);
  if ( func_buf != 0 ) {
    return (*func_buf->func)(frame);
  }
  return frame;
}

