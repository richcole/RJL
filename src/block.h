Fixnum is_block(Object *obj) {
  return get(obj, Parent) == Block;
}

Object* new_block(Object *lexical_parent, Object *code) {
  Object *block = new_object();
  set(block, Code, code);
  set(block, LexicalParent, lexical_parent);
  return block;
}


