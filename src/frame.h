
Object* new_frame() {
  Object *frame = new_object();
  set(frame, Stack, new_array());
  return frame;
}

Object* new_frame(Object *self, Object *code, Object *ret_frame) {
  Object *frame = new_object();
  Object *local = new_object();

  set(frame, Return,        ret_frame);
  set(frame, Code,          code);
  set(frame, Stack,         new_array());
  set(frame, Local,         new_object());
  set(frame, Pc,            0);
  set(local, Self,          self);
  return frame;
}

void set_lexical_parent(Object *frame, Object *parent) {
  set(get(frame, Local), Parent, parent);
}

Object* get_code(Object *frame, Fixnum index) {
  Object *code = get(frame, Code);
  return get_at(code, index);
}

Fixnum get_code_fixnum(Object *frame, Fixnum index) {
  Object *code = get(frame, Code);
  return fixnum(get_at(code, index));
}

