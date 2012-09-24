#include <string.h>

#include "char_array.h"
#include "symbols.h"

fixnum new_char_array(cxt_t *cxt, char const* s) {
  fixnum obj = new_obj(cxt);
  fixnum len = strlen(s)+1;
  set_buf(cxt, obj, s, len);
  set_buf_tail(cxt, obj, len-1);
  set(cxt, obj, SYM_TYPE, SYM_STRING);
  return obj;
}

fixnum new_char_array(cxt_t *cxt) {
  fixnum obj = new_obj(cxt);
  buf_reserve(cxt, obj, 10);
  return obj;
}

fixnum char_array_len(cxt_t *cxt, fixnum string) {
  return get_buf_tail(cxt, string);
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
  if ( tail+1 >= len ) {
    buf_reserve(cxt, string, tail*2);
  }
  char *buf = get_buf(cxt, string);
  buf[tail] = c;
  buf[++tail] = 0;
  set_buf_tail(cxt, string, tail);
}

char* char_array_get_buf(cxt_t *cxt, fixnum string) {
  return get_buf(cxt, string);
}

fixnum char_array_append(cxt_t *cxt, fixnum a, fixnum b) {
  fixnum n = new_char_array(cxt);
  fixnum a_tail = get_buf_tail(cxt, a);
  fixnum b_tail = get_buf_tail(cxt, b);
  buf_reserve(cxt, n, a_tail + b_tail + 1);
  char *n_buf = get_buf(cxt, n);
  memcpy(n_buf, get_buf(cxt, a), a_tail);
  memcpy(n_buf + a_tail, get_buf(cxt, b), b_tail);
  n_buf[a_tail + b_tail] = 0;
  set_buf_tail(cxt, n, a_tail + b_tail);
  return n;
}

  



