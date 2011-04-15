
Object* new_frame() {
  Object *frame = new_object();
  set(frame, Stack, new_array());
  return frame;
}

Object* new_closure(Object *block, Object *local, Object *self) {
  Object* closure = new_object(block);
  set(closure, "lexical_parent", local);
  set(closure, "self", self);
  return closure;
}

void set_lexical_parent(Object *frame, Object *parent) {
  set(get(frame, Local), Parent, parent);
}

Object* get_lexical_parent(Object *frame) {
  return get(get(frame, Local), Parent);
}

Object* new_local() {
  Object *local = new_object();
  set(local, "is_local", True);
  return local;
}

Object* new_frame(Object *self, Object *code, Object *ret_frame) {
  Object *frame = new_object();
  Object *local = new_local();

  if ( self == Undefined ) {
    self = get(code, "self");
  }

  set(frame, Return,        ret_frame);
  set(frame, Code,          code);
  set(frame, Stack,         new_array());
  set(frame, Local,         local);
  set(frame, Pc,            0);
  set(local, Self,          self);

  Object *lexical_parent = get(code, "lexical_parent");
  if ( lexical_parent == Undefined ) {
    set_lexical_parent(frame, get_lexical_parent(ret_frame));
  }
  else {
    set_lexical_parent(frame, lexical_parent);
  }

  Object *non_local_return = get(code, "non_local_return");
  if ( non_local_return != Undefined ) {
    set(frame, "non_local_return", non_local_return);
  }
  else {
    set(frame, "non_local_return", get(frame, "return"));
  }

  return frame;
}

Object* get_code(Object *frame, Fixnum index) {
  Object *code = get(frame, Code);
  return get_at(code, index);
}

Fixnum get_code_fixnum(Object *frame, Fixnum index) {
  Object *code = get(frame, Code);
  return fixnum(get_at(code, index));
}

Object *get_self(Object *frame) {
  return get(get(frame, Local), Self);
}


