
Object* get_code(Object *frame, Fixnum index) {
  Object *code = get(frame, Code);
  return get_at(code, index);
}

Object* get_code_fixnum(Object *frame, Fixnum index) {
  Object *code = get(frame, Code);
  return fixnum(get_at(code, index));
}

