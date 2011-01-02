
#include "fixnum.h"
#include "mem.h"
#include "object.h"
#include "string.h"
#include "array.h"
#include "symbol.h"
#include "func.h"
#include "block.h"
#include "native.h"

void init() {
  Nil->buffer = (Buffer *) new_string_buffer("Nil");
  Dirty->buffer = (Buffer *) new_string_buffer("Dirty");
  String->buffer = (Buffer *) new_string_buffer("String");
  Array->buffer = (Buffer *) new_string_buffer("Array");
  Func->buffer = (Buffer *) new_string_buffer("Func");
}

Object *get_code(Object *frame, Fixnum pc) {
  return get_at(get(frame, Code), pc);
}

Fixnum get_code_fixnum(Object *frame, Fixnum pc) {
  return fixnum(get_code(frame, pc));
}

Fixnum is_block(Object *obj) {
  return get(obj, Parent) == Block;
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

Object* new_block(Object *lexical_parent) {
  Object *block = new_object();
  Object *code  = new_array();
  set(block, Code, code);
  return block;
}

Object *resolve(Object *target, Object *slot) {
  if ( target == 0 ) {
    return 0;
  }
  Object *value = get(target, slot);
  if ( value == 0 ) {
    value = resolve(get(target, Parent), slot);
  }
  return value;
}

Object* send(Object *frame, Object *slot) {
  Object *stack  = get(frame, Stack);
  Object *target = pop(stack);
  Object *value  = resolve(target, slot);
  if ( is_block(value) ) {
    return new_frame(target, value, frame);
  }
  else if ( is_func(value) ) {
    return call_func(frame, value);
  }
  else {
    push(stack, value);
    return frame;
  }
};

Object* ret(Object *frame) {
  Object *stack        = get(frame, Stack);
  Object *ret_value    = pop(stack);
  Object *parent_frame = get(frame, Parent);
  Object *parent_stack = get(parent_frame, Stack);
  push(parent_stack, ret_value);
  return parent_frame;
}

void interp(Object *frame) {
  Fixnum pc     = 0;
  Object *instr = 0;

  while(frame != 0) {
    pc    = get_fixnum(frame, Pc);
    instr = get_code(frame, pc);

    if ( instr == Push ) {
      push(get(frame, Stack), get_code(frame, pc+1));
      pc += 2;
      set_fixnum(frame, Pc, pc);
    }

    if ( instr == Arg ) {
      set(get(frame, Local), get_code(frame, pc+1), pop(get(get(frame, Parent), Stack)));
      pc += 2;
      set_fixnum(frame, Pc, pc);
    }

    if ( instr == Self ) {
      push(get(frame, Stack), get(frame, Local));
      pc += 1;
      set_fixnum(frame, Pc, pc);
    }

    if ( instr == Send ) {
      frame = send(frame, get_code(frame, pc+1));
      pc += 2;
      set_fixnum(frame, Pc, pc);
    }

    if ( instr == Jmp ) { 
      pc += get_code_fixnum(frame, pc+1);
      set_fixnum(frame, Pc, pc);
    }

    if ( instr == JmpZ ) {
      if ( pop(get(frame, Stack)) == 0 ) {
        pc += get_code_fixnum(frame, pc+1);
      }
      else {
        pc += 2;
      }
      set_fixnum(frame, Pc, pc);
    }

    if ( instr == JmpNZ ) {
      if ( pop(get(frame, Stack)) == 0 ) {
        pc += get_code_fixnum(frame, pc+1);
      }
      else {
        pc += 2;
      }
      set_fixnum(frame, Pc, pc);
    }

    if ( instr == Ret ) {
      frame = ret(frame);
    }

    if ( instr == Term ) {
      return;
    }
  }
}

int main() {
  init();

  Object *lexical_parent = new_object();
  Object *local = new_object();
  set(lexical_parent, Local, local);

  Object *block = new_block(lexical_parent);
  Object *code  = get(block, Code);

  Object *sys = new_object();
  set(sys, Print, new_func(native_print));

  Object *parent_block = new_block(lexical_parent);
  Object *parent_frame = new_frame(0, parent_block, 0);
  Object *parent_code = get(parent_block, Code);

  push(parent_code, Ret);

  Object *frame = new_frame(0, block, parent_frame);

  Object *parent_stack = get(parent_frame, Stack);
  push(parent_stack, sys);

  push(code, Arg);
  push(code, Sys);

  push(code, Push);
  push(code, new_string("Hello World"));

  push(code, Self);
  push(code, Send);
  push(code, Sys);

  push(code, Send);
  push(code, Print);

  push(code, Ret);

  interp(frame);
};
