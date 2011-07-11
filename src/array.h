#ifndef ARRAY_H
#define ARRAY_H

#include "object.h"

struct ArrayBuffer {
  Fixnum  type;
  Fixnum  length;
  Fixnum  tail;
  Object  *data[0];
};

Object* new_array(Object *cxt);
Object* new_array_no_register(Object *cxt);
Fixnum array_length(Object *cxt, Object *array);
void push(Object *cxt, Object *array, Object *value);
Object *pop(Object *cxt, Object *array);
void push_slot(Object *cxt, Object *obj, Object *slot, Object *val);
Object *array_last(Object *cxt, Object *array);
void push_slot(Object *cxt, Object *obj, char const *s, Object *value);
Object* get_at(Object *cxt, Object *array, Fixnum index);
void set_at(Object *cxt, Object *array, Fixnum index, Object *val);
Fixnum is_array(Object *cxt, Object *array);
Fixnum is_child_array(Object *cxt, Object *array);


#endif
