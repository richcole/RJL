Object* send(Object *frame, Object *slot) {
  Object *stack  = get(frame, Stack);
  Object *target = pop(stack);
  Object *value  = 0;

  if ( is_nocall_slot(slot) ) {
    value = get(target, get_nocall_slot(slot));
    push(stack, value);
    return frame;
  }
  else  {
    value = get(target, slot);
  };

  if ( is_block(value) ) {
    return new_frame(target, value, frame);
  }
  else if ( is_func(value) ) {
    return call_func(target, value, frame);
  }
  else if ( value == Undefined && is_setter_slot(slot) ) {
    Object *field = setter_field(slot);
    if ( get(target, "is_local") == True ) {
      Object *parent = target;
      while( get(parent, "is_local") == True && get_plain(parent, field) == Undefined ) {
        parent = get(parent, "parent");
      }
      if ( get(parent, "is_local") == True ) {
        set(parent, field, pop(stack));
      }
      else {
        set(target, field, pop(stack));
      }
    }
    else {
      set(target, field, pop(stack));
    }
    return frame;
  }
  else {
    push(stack, value);
    return frame;
  }
};

Object* ret(Object *frame, Object *ret_slot) {
  Object *stack        = get(frame, Stack);
  Object *ret_value    = pop(stack);
  Object *return_frame = get(frame, ret_slot);
  if ( exists(return_frame) ) {
    Object *return_stack = get(return_frame, Stack);
    push(return_stack, ret_value);
  }
  return return_frame;
}

Object *new_push_block(Object *frame, Object *stack, Fixnum pc) {
  Object *block = get_code(frame, pc+1);
  Object *closure = new_closure(block, get(frame, Local), get_self(frame));
  set(closure, "non_local_return", pop(stack));
  return closure;
};

void advance_pc(Object *frame, Fixnum pc, Fixnum incr) {
  pc += incr;
  set_fixnum(frame, Pc, pc);
};

void interp(Object *frame) {
  Fixnum pc     = 0;
  Object *instr = 0;
  Object *new_frame = 0;

  while(frame != 0) {

    pc    = get_fixnum(frame, Pc);
    instr = get_code(frame, pc);

    if ( instr == Push ) {
      push(get(frame, Stack), get_code(frame, pc+1));
      advance_pc(frame, pc, 2);
      continue;
    }

    if ( instr == PushFrame ) {
      push(get(frame, Stack), frame);
      advance_pc(frame, pc, 1);
      continue;
    }

    if ( instr == PushBlock ) {
      Object *stack = get(frame, "stack");
      push(stack, new_push_block(frame, stack, pc));
      advance_pc(frame, pc, 2);
      continue;
    }

    if ( instr == Arg ) {
      set(get(frame, Local), 
          get_code(frame, pc+1), pop(get(get(frame, Return), Stack)));
      advance_pc(frame, pc, 2);
      continue;
    }

    if ( instr == Self ) {
      push(get(frame, Stack), get(frame, Local));
      advance_pc(frame, pc, 1);
      continue;
    }

    if ( instr == Send ) {
      new_frame = send(frame, get_code(frame, pc+1));
      if ( is_exception(new_frame) ) {
        frame = new_exception_frame(frame, new_frame);
      }
      else if ( new_frame == frame ) {
        advance_pc(frame, pc, 2);
      }
      else {
        advance_pc(frame, pc, 2);
        frame = new_frame;
      }
      continue;
    }

    if ( instr == Jmp ) { 
      pc = get_code_fixnum(frame, pc+1);
      set_fixnum(frame, Pc, pc);
      continue;
    }

    if ( instr == JmpTrue ) {
      if ( pop(get(frame, Stack)) == True ) {
        pc = get_code_fixnum(frame, pc+1);
        set_fixnum(frame, Pc, pc);
      }
      else {
        advance_pc(frame, pc, 2);
      }
      continue;
    }

    if ( instr == JmpNotTrue ) {
      if ( pop(get(frame, Stack)) != True ) {
        pc = get_code_fixnum(frame, pc+1);
        set_fixnum(frame, Pc, pc);
      }
      else {
        advance_pc(frame, pc, 2);
      }
      continue;
    }

    if ( instr == Return ) {
      frame = ret(frame, sym("non_local_return"));
      continue;
    }

    if ( instr == LocalReturn ) {
      frame = ret(frame, sym("return"));
      continue;
    }

    if ( instr == Term ) {
      return;
    }

    abort();
  }
}

