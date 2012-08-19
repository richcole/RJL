#include "native_call.h"

fixnum new_native_call(cxt_t *cxt, native_call call) {
  fixnum obj = new_obj(cxt);
  fixnum len = sizeof(native_call);
  set_buf(cxt, obj, (char *)&call, len);
  return obj;
}

native_call get_native_call(cxt_t *cxt, fixnum obj)
{
  char *buf = get_buf(cxt, obj);
  fixnum buf_len = get_buf_len(cxt, obj);
  if ( buf == 0 || buf_len < (fixnum)sizeof(native_call) ) {
    return 0;
  }
  else {
    return *(native_call*)get_buf(cxt, obj);
  }
};

