#include "frame.h"
#include "sym.h"
#include "array.h"
#include "context.h"
#include "boxed_int.h"

Object* new_frame(Object *cxt) {
  Object *frame = new_object(cxt, "Frame");
  set(cxt, frame, "stack", new_array(cxt));
  return frame;
}

Object* new_closure(Object *cxt, Object *block, Object *local, Object *self) {
  Object* closure = new_object(cxt, block);
  set(cxt, closure, "lexical_parent", local);
  set(cxt, closure, "self", self);
  return closure;
}

void set_lexical_parent(Object *cxt, Object *frame, Object *parent) {
  set(cxt, get(cxt, frame, "local"), "parent", parent);
}

Object* get_lexical_parent(Object *cxt, Object *frame) {
  return get(cxt, get(cxt, frame, "local"), "parent");
}

Object* new_local(Object *cxt) {
  Object *local = new_object(cxt, "Object");
  set(cxt, local, "is_local", get_true(cxt));
  return local;
}

Object* new_frame(Object *cxt, Object *self, Object *code, Object *ret_frame) {
  Object *frame = new_object(cxt, "Frame");
  Object *local = new_local(cxt);

  if ( ! exists(cxt, self) ) {
    self = get(cxt, code, "self");
  }

  set(cxt, frame, "return",        ret_frame);
  set(cxt, frame, "code",          code);
  set(cxt, frame, "stack",         new_array(cxt));
  set(cxt, frame, "local",         local);
  set(cxt, frame, "pc",            new_boxed_int(cxt, 0));
  set(cxt, local, "self",          self);

  Object *lexical_parent = get(cxt, code, "lexical_parent");
  if ( exists(cxt, lexical_parent) ) {
    set_lexical_parent(cxt, frame, lexical_parent);
  }
  else {
    set_lexical_parent(cxt, frame, get_lexical_parent(cxt, ret_frame));
  }

  Object *non_local_return = get(cxt, code, "non_local_return");
  if ( exists(cxt, non_local_return) ) {
    set(cxt, frame, "non_local_return", non_local_return);
  }
  else {
    set(cxt, frame, "non_local_return", get(cxt, frame, "return"));
  }

  return frame;
}

Object* get_code(Object *cxt, Object *frame, Fixnum index) {
  Object *code = get(cxt, frame, "code");
  return get_at(cxt, code, index);
}

Fixnum get_code_fixnum(Object *cxt, Object *frame, Fixnum index) {
  Object *code = get(cxt, frame, "code");
  return boxed_int_to_fixnum(cxt, get_at(cxt, code, index));
}

Object *get_self(Object *cxt, Object *frame) {
  return get(cxt, get(cxt, frame, "local"), "self");
}

Object *get_stack(Object *cxt, Object *frame) {
  return get(cxt, frame, "stack");
}

