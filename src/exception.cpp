#include "exception.h"
#include "sym.h"
#include "array.h"
#include "context.h"
#include "char_array.h"
#include "frame.h"

Object *get_catch_block(Object *cxt, Object *frame) {
  return get(cxt, get(cxt, frame, "code"), "catch");
}

Object *find_catch_frame(Object *cxt, Object *frame) {
  while( exists(cxt, frame) ) {
    if ( exists(cxt, get_catch_block(cxt, frame)) ) {
      return frame;
    }
    frame = get(cxt, frame, "non_local_return");
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
  Object *ret_frame = get(cxt, catch_frame, "return");
  Object *self = get_self(cxt, catch_frame);
  Object *catch_block = get_catch_block(cxt, catch_frame);
  Object *new_catch_frame = new_frame(cxt, self, catch_block, ret_frame);
  Object *old_local = get(cxt, catch_frame, "local");
  set_lexical_parent(cxt, new_catch_frame, old_local);
  push(cxt, get(cxt, ret_frame, "stack"), ex);
  return new_catch_frame;
}

Object *new_exception(Object *cxt, Object *frame, Object *reason) {
  Object *ex = new_object(cxt, context_get(cxt, "Exception"));
  set(cxt, ex, "frame",  frame);
  set(cxt, ex, "reason", reason);
  return ex;
}

Object *new_exception(Object *cxt, Object *frame, char const* reason) {
  return new_exception(cxt, frame, new_char_array(cxt, reason));
}

void init_exception_sys(Object *cxt) {
};


