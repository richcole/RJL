#include "block.h"
#include "sym.h"
#include "array.h"
#include "context.h"

Fixnum is_block(Object *cxt, Object *obj) {
  return is_true(cxt, get(cxt, obj, "is_block"));
}

Object* new_block(Object *cxt) {
  Object *block = new_array(cxt);
  set_true(cxt, block, "is_block");
  set(cxt, block, "parent", context_get(cxt, "Block"));
  return block;
}



