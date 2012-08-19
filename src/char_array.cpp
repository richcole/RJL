#include "char_array.h"
#include <string.h>

fixnum new_char_array(cxt_t *cxt, char const* s) {
  fixnum obj = new_obj(cxt);
  fixnum len = strlen(s)+1;
  set_buf(cxt, obj, s, len);
  return obj;
}

fixnum new_char_array(cxt_t *cxt) {
  fixnum obj = new_obj(cxt);
  buf_reserve(cxt, obj, 10);
  return obj;
}

fixnum char_array_get_at(cxt_t *cxt, fixnum string, fixnum index) {
  fixnum len = get_buf_len(cxt, string);
  if ( index >= len ) {
    return 0;
  }
  else {
    return get_buf(cxt, string)[index];
  }
}

void char_array_put_char(cxt_t *cxt, fixnum string, fixnum c) {
  fixnum len = get_buf_len(cxt, string);
  fixnum tail = get_buf_tail(cxt, string);
  if ( tail == len ) {
    buf_reserve(cxt, string, tail*2);
  }
  get_buf(cxt, string)[tail] = c;
  set_buf_tail(cxt, string, tail+1);
}

char* char_array_get_string(cxt_t *cxt, fixnum string) {
  return get_buf(cxt, string);
}

  



