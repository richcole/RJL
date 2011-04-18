#include "block.h"
#include "sym.h"
#include "array.h"

Fixnum is_block(Object *cxt, Object *obj) {
  return is_true(cxt, get(cxt, obj, "is_block"));
}

Object* new_block(Object *cxt) {
  Object *block = new_array(cxt);
  set_true(cxt, block, "is_block");
  return block;
}



