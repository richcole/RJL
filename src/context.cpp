#include "context.h"
#include "std.h"
#include "symbol_table.h"
#include "file.h"
#include "native.h"
#include "parser.h"
#include "scanner.h"
#include "code_generator.h"

Object* new_context() {
  Object *cxt = new_symbol_table(0);
  Object *undefined_sym = sym(cxt, "undefined");
  set(cxt, undefined_sym, undefined_sym);

  Object *object = new_object(cxt);
  context_set(cxt, "Object", object);

  init_file_sys(cxt);
  init_native_sys(cxt);
  init_scanner_sys(cxt);
  init_parser_sys(cxt);
  init_code_generator_sys(cxt);
  return cxt;
}
  
Object* context_get(Object *cxt, char const* slot) {
  Object *obj = get(cxt, cxt, slot);
  if ( is_undefined(cxt, obj) ) {
    context_set(cxt, slot, obj = new_object(cxt, "Object"));
  }
  return obj;
};

void context_set(Object *cxt, char const* slot, Object *value) {
  return set(cxt, sym(cxt, slot), value);
};

Object* get_symbol_table(Object *cxt) {
  return cxt;
}

