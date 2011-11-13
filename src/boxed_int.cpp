#include "boxed_int.h"
#include "mem.h"
#include "sym.h"
#include "char_array.h"
#include "type_tags.h"
#include "context.h"

BoxedIntBuffer *new_boxed_int_buffer(Object *cxt, long value) {
  BoxedIntBuffer *buf = (BoxedIntBuffer *)context_alloc_buffer(cxt, sizeof(BoxedIntBuffer));
  buf->type   = BoxedIntTypeTag;
  buf->value  = value;
  return buf;
}

Object *new_boxed_int(Object *cxt) {
  Object *boxed_int = new_object(cxt, "BoxedInt");
  boxed_int->buffer = (Buffer *) new_boxed_int_buffer(cxt, 0);
  return boxed_int;
}

Object *new_boxed_int(Object *cxt, long value) {
  Object *boxed_int = new_object(cxt, "BoxedInt");
  boxed_int->buffer = (Buffer *) new_boxed_int_buffer(cxt, value);
  return boxed_int;
}

def_get_buffer(BoxedInt, boxed_int, BoxedIntTypeTag);
def_set_buffer(BoxedInt, boxed_int, BoxedIntTypeTag);

Fixnum is_boxed_int(Object *cxt, Object *boxed_int) {
  if ( get_boxed_int_buffer(boxed_int) != 0 ) {
    return 1;
  }
  return 0;
}

Fixnum boxed_int_equals(Object *a, Object *b) {
  BoxedIntBuffer *ab = get_boxed_int_buffer(a);
  BoxedIntBuffer *bb = get_boxed_int_buffer(b);
  if (ab == 0 || bb == 0) {
    return 0;
  }
  return ab->value == bb->value;
}
  

Object *char_array_to_boxed_int(Object *cxt, Object *str) {
  Object *boxed_int = new_boxed_int(cxt);
  BoxedIntBuffer  *int_buf = get_boxed_int_buffer(boxed_int);
  CharArrayBuffer *str_buf = get_char_array_buffer(str);
  Fixnum sign = 1;
  Fixnum i = 0;

  if ( str_buf == 0 || int_buf == 0 ) {
    return get_undefined(cxt);
  }
  if ( str_buf->data[i] == '-' ) {
    sign = -1;
    ++i;
  }
  if ( str_buf->data[i] == '0' && str_buf->data[i+1] == 'x' ) {
    i+=2;
    while( is_hexdigit(str_buf->data[i]) ) {
      int_buf->value *= 16;
      if ( is_digit(str_buf->data[i]) ) {
        int_buf->value += (str_buf->data[i] - '0');
      }
      else if ( str_buf->data[i] >= 'a' || str_buf->data[i] <= 'f' ) {
        int_buf->value += (str_buf->data[i] - 'a');
      }
      else if ( str_buf->data[i] >= 'A' || str_buf->data[i] <= 'F' ) {
        int_buf->value += (str_buf->data[i] - 'A');
      }
      ++i;
    }
    int_buf->value *= sign;
  }
  else {
    while( is_digit(str_buf->data[i]) ) {
      int_buf->value *= 10;
      int_buf->value += (str_buf->data[i] - '0');
      ++i;
    }
    int_buf->value *= sign;
  }

  return boxed_int;
}

Fixnum boxed_int_to_fixnum(Object *cxt, Object *boxed_int) {
  BoxedIntBuffer *buf = get_boxed_int_buffer(boxed_int);
  if ( buf != 0 ) {
    return buf->value;
  }
  else {
    return 0;
  }
}


