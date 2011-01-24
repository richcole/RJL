
struct ArrayBuffer {
  Object  *type;
  Fixnum  length;
  Fixnum  tail;
  Object  *data[0];
};

Object* Array = new_object();
Object* ArrayObject = new_object();

ArrayBuffer *new_array_buffer(int len) {
  ArrayBuffer *buf = (ArrayBuffer *)mem_alloc(sizeof(ArrayBuffer)+(len*sizeof(Object *)));
  buf->type   = Array;
  buf->length = len;
  buf->tail   = 0;
  return buf;
}

Object* new_array() {
  Object *array = new_object();
  array->buffer = (Buffer *) new_array_buffer(10);
  set(array, Parent, ArrayObject);
  return array;
}

def_get_buffer(Array, array);
def_set_buffer(Array, array);

Fixnum array_length(Object *array) {
  ArrayBuffer *buf = get_array_buffer(array);
  if ( buf != 0 ) {
    return buf->tail;
  }
  return 0;
}

Fixnum is_array(Object *array) {
  if ( get_array_buffer(array) != 0 ) {
    return 1;
  }
  return 0;
}

Object* get_at(Object *array, Fixnum index) {
  ArrayBuffer *array_buffer = get_array_buffer(array);
  if ( array_buffer != 0 ) {
    if ( index < array_buffer->tail ) {
      return array_buffer->data[index];
    }
  }
  return 0;
}

void grow_array(Object *array) {
  ArrayBuffer *array_buffer = get_array_buffer(array);
  if ( array_buffer != 0 ) {
    ArrayBuffer *new_buffer = new_array_buffer(array_buffer->length*2);
    rjl_memcpy(new_buffer->data, array_buffer->data, 
      array_buffer->length * sizeof(Object *)
    );
    new_buffer->tail = array_buffer->tail;
    set_array_buffer(array, new_buffer);
    mem_free(array_buffer);
  }
}

void push(Object *array, Object *value) {
  ArrayBuffer *array_buffer = get_array_buffer(array);
  if ( array_buffer != 0 ) {
    if ( array_buffer->tail >= array_buffer->length ) {
      grow_array(array);
      array_buffer = get_array_buffer(array);
    }
    array_buffer->data[array_buffer->tail++] = value;
  }
}

Object *pop(Object *array) {
  ArrayBuffer *array_buffer = get_array_buffer(array);
  if ( array_buffer != 0 ) {
    if ( array_buffer->tail > 0 ) {
      return array_buffer->data[--array_buffer->tail];
    }
  }
  return 0;
}

void push_slot(Object *obj, Object *slot, Object *val) {
  Object *stack = get(obj, slot);
  if ( ! exists(stack) ) {
    stack = new_array();
    set(obj, slot, stack);
  }
  push(stack, val);
}

void init_array_symbols() {
	add_sym(Array, "Array");
}




