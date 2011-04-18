#include "array.h"
#include "context.h"
#include "mem.h"
#include "buffer.cpp"
#include "sym.h"
#include "type_tags.h"

ArrayBuffer *new_array_buffer(Object *cxt, int len) {
  create_new_buffer(Array, array, ArrayTypeTag, len*sizeof(Object *));
  buf->length = len;
  buf->tail   = 0;
  return buf;
}

Object* new_array(Object *cxt) {
  Object *array = new_object(cxt, context_get(cxt, "Array"));
  array->buffer = (Buffer *) new_array_buffer(cxt, 10);
  return array;
}

def_get_buffer(Array, array, ArrayTypeTag);
def_set_buffer(Array, array, ArrayTypeTag);

Fixnum array_length(Object *cxt, Object *array) {
  ArrayBuffer *buf = get_array_buffer(array);
  if ( buf != 0 ) {
    return buf->tail;
  }
  return 0;
}

Fixnum is_array(Object *cxt, Object *array) {
  if ( get_array_buffer(array) != 0 ) {
    return 1;
  }
  return 0;
}

Object* get_at(Object *cxt, Object *array, Fixnum index) {
  ArrayBuffer *array_buffer = get_array_buffer(array);
  if ( array_buffer != 0 ) {
    if ( index < array_buffer->tail ) {
      return array_buffer->data[index];
    }
  }
  return 0;
}

void set_at(Object *cxt, Object *array, Fixnum index, Object *val) {
  ArrayBuffer *array_buffer = get_array_buffer(array);
  if ( array_buffer != 0 ) {
    if ( index < array_buffer->tail ) {
      array_buffer->data[index] = val;
    }
  }
}

void grow_array(Object *cxt, Object *array) {
  ArrayBuffer *array_buffer = get_array_buffer(array);
  if ( array_buffer != 0 ) {
    ArrayBuffer *new_buffer = new_array_buffer(cxt, array_buffer->length*2);
    rjl_memcpy(new_buffer->data, array_buffer->data, 
      array_buffer->length * sizeof(Object *)
    );
    new_buffer->tail = array_buffer->tail;
    set_array_buffer(array, new_buffer);
    mem_free(array_buffer);
  }
}

void push(Object *cxt, Object *array, Object *value) {
  ArrayBuffer *array_buffer = get_array_buffer(array);
  if ( array_buffer != 0 ) {
    if ( array_buffer->tail >= array_buffer->length ) {
      grow_array(cxt, array);
      array_buffer = get_array_buffer(array);
    }
    array_buffer->data[array_buffer->tail++] = value;
  }
}

Object *pop(Object *cxt, Object *array) {
  ArrayBuffer *array_buffer = get_array_buffer(array);
  if ( array_buffer != 0 ) {
    if ( array_buffer->tail > 0 ) {
      return array_buffer->data[--array_buffer->tail];
    }
  }
  return 0;
}

Object *array_last(Object *cxt, Object *array) {
  ArrayBuffer *array_buffer = get_array_buffer(array);
  if ( array_buffer != 0 ) {
    if ( array_buffer->tail > 0 ) {
      return array_buffer->data[array_buffer->tail-1];
    }
  }
  return 0;
}

void push_slot(Object *cxt, Object *obj, Object *slot, Object *val) {
  Object *stack = get(cxt, obj, slot);
  if ( ! exists(cxt, stack) ) {
    stack = new_array(cxt);
    set(obj, slot, stack);
  }
  push(cxt, stack, val);
}

void push_slot(Object *cxt, Object *obj, char const *s, Object *value) {
  push_slot(cxt, obj, sym(cxt, s), value);
}

