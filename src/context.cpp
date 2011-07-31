#include "context.h"
#include "std.h"
#include "symbol_table.h"
#include "file.h"
#include "native.h"
#include "native_sdl.h"
#include "parser.h"
#include "scanner.h"
#include "code_generator.h"
#include "gc.h"

Object* new_context() {
  Object *cxt              = new_symbol_table_no_register();
  Object *undefined_sym    = new_char_array_no_register(cxt, "undefined");
  Object *parent_sym       = new_char_array_no_register(cxt, "parent");
  Object *object_sym       = new_char_array_no_register(cxt, "Object");
  Object *char_array_sym   = new_char_array_no_register(cxt, "CharArray");
  Object *objects_sym      = new_char_array_no_register(cxt, "objects");
  Object *objects          = new_array_no_register(cxt);

  add_sym(cxt, undefined_sym);
  add_sym(cxt, parent_sym);
  add_sym(cxt, objects_sym);
  add_sym(cxt, char_array_sym);
  add_sym(cxt, object_sym);

  set(cxt, cxt, undefined_sym, undefined_sym);
  set(cxt, cxt, objects_sym, objects);

  context_register_object(cxt, undefined_sym);
  context_register_object(cxt, parent_sym);
  context_register_object(cxt, char_array_sym);
  context_register_object(cxt, object_sym);
  context_register_object(cxt, objects_sym);
  context_register_object(cxt, objects);

  Object *object = new_object(cxt);
  set(cxt, cxt, object_sym, object);

  Object *char_array = new_object(cxt);
  set(cxt, cxt, char_array_sym, char_array);

  Object *char_array_object = context_get(cxt, "CharArray");
  set(cxt, objects_sym,    "parent", char_array_object);
  set(cxt, undefined_sym,  "parent", char_array_object);
  set(cxt, char_array_sym, "parent", char_array_object);
  set(cxt, parent_sym,     "parent", char_array_object);
  set(cxt, objects,        "parent", context_get(cxt, "Array"));
  set(cxt, cxt,            "parent", object);

  init_exception_sys(cxt);
  init_file_sys(cxt);
  init_native_sys(cxt);
  init_native_sdl_sys(cxt);
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
  return set(cxt, cxt, sym(cxt, slot), value);
};

Object* get_symbol_table(Object *cxt) {
  return cxt;
}

void* context_alloc_buffer(Object *cxt, Fixnum length) {
  return (Object *)mem_alloc(length);
}

void context_free_buffer(Object *cxt, void *buf) {
  mem_free(buf);
}

void context_register_object(Object *cxt, Object *obj) {
  Object *objects = context_get(cxt, "objects");
  set(cxt, objects, obj, obj);
}

void context_unregister_object(Object *cxt, Object *obj) {
  Object *objects = context_get(cxt, "objects");
  unset(cxt, objects, obj);
}

void context_mark(Object *cxt, Object *obj, Object *mark, Object *objects) 
{
  if ( obj == 0 ) return;
  if ( obj->mark == mark ) return;
  if ( obj == objects ) return;
  obj->mark = mark;
  for(Fixnum i=0;i<obj->length; ++i) {
    if ( obj->table[i].key != 0 && obj->table[i].key != DirtyKey ) {
      context_mark(cxt, obj->table[i].key, mark, objects);
      context_mark(cxt, obj->table[i].value, mark, objects);
    }
  }
  if ( is_child_array(cxt, obj) ) {
    Fixnum len = array_length(cxt, obj);
    for(Fixnum i=0;i<len; ++i) {
      context_mark(cxt, get_at(cxt, obj, i), mark, objects);
    }
  }
  SymbolTableBuffer *stb = get_symbol_table_buffer(obj);
  if ( stb != 0 ) {
    for(Fixnum i=0;i<stb->length;++i) {
      context_mark(cxt, stb->data[i], mark, objects);
    }
  }
}

Fixnum context_mark_and_sweep(Object *cxt, Object *frame) {
  Object *mark = new_object(cxt);
  Object *objects = context_get(cxt, "objects");
  context_mark(cxt, cxt, mark, objects);
  context_mark(cxt, frame, mark, objects);
  Fixnum objects_released = 0;
  for(Fixnum i=0;i<objects->length; ++i) {
    Object *key = objects->table[i].key;
    if ( key != 0 && key != DirtyKey && key != objects && key != mark && key->mark != mark) {
      object_dispose(cxt, key);
      objects->table[i].key = DirtyKey;
      objects->table[i].value = 0;
      objects->occupied--;
      ++objects_released;
    }
  }
  return objects_released;
}
