#ifndef BOXED_INT_H
#define BOXED_INT_H

#include "object.h"

struct BoxedIntBuffer {
  Object *type;
  long   value;
};

decl_get_buffer(BoxedInt, boxed_int);
decl_set_buffer(BoxedInt, boxed_int);

Object *new_boxed_int(Object *cxt);
Object *new_boxed_int(Object *cxt, long value);
Fixnum is_boxed_int(Object *cxt, Object *boxed_int);
Fixnum boxed_int_to_fixnum(Object *cxt, Object *boxed_int);
Fixnum get_fixnum(Object *cxt, Object *obj, char const* slot);
void set_fixnum(Object *cxt, Object *obj, char const* slot, Fixnum value);
Object *char_array_to_boxed_int(Object *cxt, Object *str);


#endif
