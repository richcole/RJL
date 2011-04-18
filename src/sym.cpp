#include "sym.h"
#include "symbol_table.h"
#include "context.h"
#include "array.h"
#include "boxed_int.h"

Object *sym(Object *cxt, char const* str) {
  return symbol_table_add(grow_symbol_table(get_symbol_table(cxt)), str); 
}

Object *sym(Object *cxt, Object *str) {
	return symbol_table_add(grow_symbol_table(get_symbol_table(cxt)), str); 
}

void set(Object *cxt, Object *target, char const* s, Object *value) {
  set(target, sym(cxt, s), value);
}

void set(Object *cxt, Object *target, char const* s, char const *v) {
  set(target, sym(cxt, s), sym(cxt, v));
}

Object *get(Object *cxt, Object *target, char const* s) {
  return get(cxt, target, sym(cxt, s));
}

Object *get_undefined(Object *cxt) {
  return context_get(cxt, "undefined");
}

void push_sym(Object *cxt, Object *stack, char const* v) {
  push(cxt, stack, sym(cxt, v));
}

Fixnum is_true(Object *cxt, Object *obj) {
  return obj == context_get(cxt, "true");
}

Fixnum is_false(Object *cxt, Object *obj) {
  return obj == context_get(cxt, "false");
}

Object* get_true(Object *cxt) {
  return context_get(cxt, "true");
};

Object* get_false(Object *cxt) {
  return context_get(cxt, "false");
};

void set_slot_true(Object *cxt, Object *obj, Object *slot) {
  set(obj, slot, get_true(cxt));
};

void set_slot_true(Object *cxt, Object *obj, char const* slot) {
  set(obj, sym(cxt, slot), get_true(cxt));
};

Fixnum is_slot_true(Object *cxt, Object *obj, char const* slot) {
  return get(cxt, obj, slot) == get_true(cxt);
};

Fixnum is_undefined(Object *cxt, Object *obj) {
  return obj == get_undefined(cxt);
}

Fixnum is_slot_undefined(Object *cxt, Object *obj, char const* slot) {
  return get(cxt, obj, slot) == get_undefined(cxt);
}

void push_undefined(Object *cxt, Object *stack) {
  push(cxt, stack, get_undefined(cxt));
}

void set_undefined(Object *cxt, Object *obj, char const* slot) {
  set(cxt, obj, slot, get_undefined(cxt));
}

Object* return_undefined(Object *cxt, Object *frame) {
  push(cxt, get(cxt, frame, "stack"), get_undefined(cxt));
  return frame;
}

Object* return_value(Object *cxt, Object *frame, Object *value) {
  push(cxt, get(cxt, frame, "stack"), value);
  return frame;
}

Fixnum get_fixnum(Object *cxt, Object *obj, char const* slot) {
  return boxed_int_to_fixnum(cxt, get(cxt, obj, slot));
}

void set_fixnum(Object *cxt, Object *obj, char const* slot, Fixnum value) {
  set(cxt, obj, slot, new_boxed_int(cxt, value));
}




