
Object* new_frame() {
  Object *frame = new_object();
  set(frame, Stack, new_array());
  return frame;
}

Object* new_frame(Object *self, Object *block, Object *parent_frame) {
  Object *frame = new_object();
  Object *local = new_object();

  set(frame, Parent,        parent_frame);
  set(frame, Stack,         new_array());
  set(frame, Code,          get(block, Code));
  set(frame, Local,         local);
  set(frame, Pc,            0);

  set(local, Self,          self);
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

