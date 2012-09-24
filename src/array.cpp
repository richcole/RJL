#include "array.h"
#include "symbols.h"

fixnum new_array(cxt_t *cxt) {
  fixnum obj = new_obj(cxt);
  array_reserve(cxt, obj, 10);
  set(cxt, obj, SYM_TYPE, SYM_ARRAY);
  return obj;
}

fixnum* array_get_buf(cxt_t *cxt, fixnum obj) {
  return (fixnum *)get_buf(cxt, obj);
}

fixnum array_get_buf_len(cxt_t *cxt, fixnum obj) {
  return get_buf_len(cxt, obj) / sizeof(fixnum);
}

fixnum array_get_buf_tail(cxt_t *cxt, fixnum obj) {
  return get_buf_tail(cxt, obj) / sizeof(fixnum);
}

void array_set_buf_tail(cxt_t *cxt, fixnum obj, fixnum tail) {
  set_buf_tail(cxt, obj, tail*sizeof(fixnum));
}

void array_reserve(cxt_t *cxt, fixnum obj, fixnum len) {
  buf_reserve(cxt, obj, len*sizeof(len));
}

fixnum array_pop(cxt_t *cxt, fixnum obj) {
  fixnum *buf = array_get_buf(cxt, obj);
  fixnum buf_tail = array_get_buf_tail(cxt, obj);
  if ( buf == 0 || buf_tail == 0 ) {
    return 0;
  }
  else {
    --buf_tail;
    array_set_buf_tail(cxt, obj, buf_tail);
    return buf[buf_tail];
  }

}


fixnum array_get_at(cxt_t *cxt, fixnum obj, fixnum pos) {
  fixnum *buf = array_get_buf(cxt, obj);
  fixnum  buf_tail = array_get_buf_tail(cxt, obj);
  if ( buf == 0 || buf_tail < pos ) {
    return 0;
  }
  return buf[pos];
}

void array_set_at(cxt_t *cxt, fixnum obj, fixnum pos, fixnum value) {
  fixnum  buf_len = array_get_buf_len(cxt, obj);
  fixnum *buf = 0;
  if ( buf_len <= pos ) {
    array_reserve(cxt, obj, pos*2);
  }
  buf = array_get_buf(cxt, obj);
  buf[pos] = value;
}

fixnum array_peek(cxt_t *cxt, fixnum obj) {
  fixnum *buf = array_get_buf(cxt, obj);
  fixnum  buf_tail = array_get_buf_tail(cxt, obj);
  if ( buf == 0 || buf_tail == 0 ) {
    return 0;
  }
  else {
    return buf[buf_tail-1];
  }
}

fixnum array_len(cxt_t *cxt, fixnum obj) {
  fixnum *buf = array_get_buf(cxt, obj);
  fixnum  buf_tail = array_get_buf_tail(cxt, obj);
  if ( buf == 0 ) {
    return 0;
  }
  else {
    return buf_tail;
  }
}

void array_push(cxt_t *cxt, fixnum obj, fixnum value) {
  fixnum len = array_get_buf_len(cxt, obj);
  fixnum tail = array_get_buf_tail(cxt, obj);
  if ( tail == len ) {
    array_reserve(cxt, obj, len*2);
  }
  array_get_buf(cxt, obj)[tail] = value;
  array_set_buf_tail(cxt, obj, tail+1);
}

