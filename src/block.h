Fixnum is_block(Object *obj) {
  return get(obj, Parent) == Block;
}

Object* new_closure(Object *parent, Object *code) {
  Object *block = new_object();
  set(block, Code, code);
  set(block, Parent, parent);
  return block;
}


