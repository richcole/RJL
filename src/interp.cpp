#include "interp.h"
#include "std.h"
#include "setter.h"
#include "block.h"
#include "frame.h"
#include "exception.h"
#include "abort.h"
#include "native.h"

Object* get_self_from_target(Object *cxt, Object *target) {
  Object *self = get(cxt, target, "self");
  if ( exists(cxt, self) ) {
    return self;
  }
  else {
    return target;
  }
};

Object* send(Object *cxt, Object *frame, Object *slot) {
  Object *stack  = get_stack(cxt, frame);
  Object *target = pop(cxt, stack);
  Object *value  = 0;

  if ( is_nocall_slot(cxt, slot) ) {
    value = get(cxt, target, get_nocall_slot(cxt, slot));
    push(cxt, stack, value);
    return frame;
  }
  else  {
    value = get(cxt, target, slot);
  };

  if ( is_block(cxt, value) ) {
    return new_frame(cxt, get_self_from_target(cxt, target), value, frame);
  }
  else if ( is_func(cxt, value) ) {
    return call_func(cxt, target, value, frame);
  }
  else if ( is_undefined(cxt, value) && is_setter_slot(cxt, slot) ) {
    Object *field     = setter_field(cxt, slot);
    Object *field_val = peek(cxt, stack);
    if ( is_slot_true(cxt, target, "is_local") ) {
      Object *parent = target;
      while ( 
        is_slot_true(cxt, parent, "is_local") && 
        get_plain(parent, field) == 0
      ) {
        parent = get(cxt, parent, "parent");
      }
      if ( is_slot_true(cxt, parent, "is_local") ) {
        set(cxt, parent, field, field_val);
      }
      else {
        set(cxt, target, field, field_val);
      }
    }
    else {
      set(cxt, target, field, field_val);
    }
    return frame;
  }
  else {
    push(cxt, stack, value);
    return frame;
  }
};

Object* ret(Object *cxt, Object *frame, Object *ret_slot) {
  Object *stack        = get_stack(cxt, frame);
  Object *ret_value    = pop(cxt, stack);
  Object *return_frame = get(cxt, frame, ret_slot);
  if ( exists(cxt, return_frame) ) {
    Object *return_stack = get(cxt, return_frame, "stack");
    push(cxt, return_stack, ret_value);
  }
  return return_frame;
}

Object *new_push_block(Object* cxt, Object *frame, Object *stack, Fixnum pc) {
  Object *block = get_code(cxt, frame, pc+1);
  Object *closure = new_closure(cxt, block, get(cxt, frame, "local"), get_self(cxt, frame));
  return closure;
};

void advance_pc(Object *cxt, Object *frame, Fixnum pc, Fixnum incr) {
  pc += incr;
  set_fixnum(cxt, frame, "pc", pc);
};

void interp(Object *cxt, Object *frame) {
  Fixnum pc     = 0;
  Object *instr = 0;
  Object *new_frame = 0;
  Fixnum instr_count = 0;

  while(frame != 0) {

    pc    = get_fixnum(cxt, frame, "pc");
    instr = get_code(cxt, frame, pc);

    ++instr_count;
    if ( instr_count > 100 ) {
      context_mark_and_sweep(cxt, frame);
      instr_count = 0;
    }

    if ( instr == sym(cxt, "push") ) {
      push(cxt, get_stack(cxt, frame), get_code(cxt, frame, pc+1));
      advance_pc(cxt, frame, pc, 2);
      continue;
    }

    if ( instr == sym(cxt, "pop") ) {
      pop(cxt, get_stack(cxt, frame));
      advance_pc(cxt, frame, pc, 1);
      continue;
    }

    if ( instr == sym(cxt, "push_block") ) {
      Object *stack = get_stack(cxt, frame);
      push(cxt, stack, new_push_block(cxt, frame, stack, pc));
      advance_pc(cxt, frame, pc, 2);
      continue;
    }

    if ( instr == sym(cxt, "arg") ) {
      Object *value = pop(cxt, get(cxt, get(cxt, frame, "return"), "stack"));
      set(cxt, get(cxt, frame, "local"), get_code(cxt, frame, pc+1), value);
      advance_pc(cxt, frame, pc, 2);
      continue;
    }

    if ( instr == sym(cxt, "self") ) {
      push(cxt, get_stack(cxt, frame), get(cxt, frame, "local"));
      advance_pc(cxt, frame, pc, 1);
      continue;
    }

    if ( instr == sym(cxt, "send") ) {
      new_frame = send(cxt, frame, get_code(cxt, frame, pc+1));
      if ( is_exception(cxt, new_frame) ) {
        frame = new_exception_frame(cxt, frame, new_frame);
      }
      else if ( new_frame == frame ) {
        advance_pc(cxt, frame, pc, 2);
      }
      else {
        advance_pc(cxt, frame, pc, 2);
        frame = new_frame;
      }
      continue;
    }

    if ( instr == sym(cxt, "jmp") ) { 
      pc = get_code_fixnum(cxt, frame, pc+1);
      set_fixnum(cxt, frame, "pc", pc);
      continue;
    }

    if ( instr == sym(cxt, "jmp_true") ) {
      if ( is_true(cxt, pop(cxt, get_stack(cxt, frame))) ) {
        pc = get_code_fixnum(cxt, frame, pc+1);
        set_fixnum(cxt, frame, "pc", pc);
      }
      else {
        advance_pc(cxt, frame, pc, 2);
      }
      continue;
    }

    if ( instr == sym(cxt, "jmp_not_true") ) {
      if ( ! is_true(cxt, pop(cxt, get_stack(cxt, frame))) ) {
        pc = get_code_fixnum(cxt, frame, pc+1);
        set_fixnum(cxt, frame, "pc", pc);
      }
      else {
        advance_pc(cxt, frame, pc, 2);
      }
      continue;
    }

    if ( instr == sym(cxt, "return") ) {
      frame = ret(cxt, frame, sym(cxt, "return"));
      continue;
    }

    if ( instr == sym(cxt, "term") ) {
      return;
    }

    abort();
  }
}

