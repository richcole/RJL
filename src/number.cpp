#include "number.h"
#include "symbols.h"

fixnum new_number(cxt_t *cxt, fixnum num) {
  fixnum obj = new_obj(cxt);
  buf_reserve(cxt, obj, sizeof(fixnum));
  set_buf(cxt, obj, (char *)&num, sizeof(fixnum));
  set(cxt, num, SYM_TYPE, SYM_NUMBER);
  return obj;
}


