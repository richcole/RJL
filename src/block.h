Object *BlockObject = new_object();

Fixnum is_block(Object *obj) {
  return get(obj, "is_block") == True;
}

Object* new_block() {
  Object *block = new_array();
  set(block, "parent", BlockObject);
  set(block, "is_block", True);
  return block;
}

Object* new_closure(Object *parent, Object *code) {
  Object *block = new_object();
  set(block, "code", code);
  set(block, "parent", parent);
  return block;
}

void init_block_symbols() {
};

void init_block_sys(Object *sys) {
};

