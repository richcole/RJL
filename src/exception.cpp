#include "exception.h"
#include "sym.h"
#include "array.h"
#include "context.h"
#include "char_array.h"

Object *find_catch_frame(Object *cxt, Object *frame) {
  while( exists(cxt, frame) ) {
    if ( array_length(cxt, get(cxt, frame, "catch")) != 0 ) {
      return frame;
    }
    frame = get(cxt, frame, "parent");
  }
  return get_undefined(cxt);
}

Fixnum is_exception(Object *cxt, Object *ex) {
  Object *parent = get_parent(cxt, ex);
  if ( parent == context_get(cxt, "Exception") ) {
    return 1;
  }
  else {
    return 0;
  }
}

Object *new_exception_frame(Object *cxt, Object *frame, Object *ex) {
  Object *catch_frame = new_object(cxt, find_catch_frame(cxt, frame));
  push(cxt, get(cxt, catch_frame, "stack"), ex);
  set(cxt, catch_frame, "pc", pop(cxt, get(cxt, catch_frame, "catch")));
  return catch_frame;
}

Object *new_exception(Object *cxt, Object *frame, char const* reason) {
  Object *ex = new_object(cxt);
  set(cxt, ex, "parent", context_get(cxt, "Exception"));
  set(cxt, ex, "frame",  frame);
  set(cxt, ex, "reason", new_char_array(cxt, reason));
  return ex;
}


