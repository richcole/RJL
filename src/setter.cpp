#include "setter.h"
#include "std.h"

Fixnum is_nocall_slot(Object *cxt, Object *value) {
  Fixnum len = char_array_length(cxt, value);
  if ( len > 0 ) {
    return char_array_get_at(cxt, value, 0) == '^';
  }
  else {
    return 0;
  }
}

Object* get_nocall_slot(Object *cxt, Object *slot) {
  Fixnum len = char_array_length(cxt, slot);
  if ( len > 0 ) {
    return sym(cxt, char_array_subchar_array(cxt, slot, 1, len));
  }
  else {
    return get_undefined(cxt);
  }
}

Fixnum is_setter_slot(Object *cxt, Object *value) {
  Fixnum len = char_array_length(cxt, value);
  if ( len > 0 ) {
    return char_array_get_at(cxt, value, len-1) == ':';
  }
  else {
    return 0;
  }
}

Object* setter_field(Object *cxt, Object *value) {
  Fixnum len = char_array_length(cxt, value);
  if ( len > 0 ) {
    return sym(cxt, char_array_subchar_array(cxt, value, 0, len-1));
  }
  else {
    return get_undefined(cxt);
  }
}
