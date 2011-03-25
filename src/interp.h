Object* send(Object *frame, Object *slot) {
  Object *stack  = get(frame, Stack);
  Object *target = pop(stack);
  Object *value  = get(target, slot);
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

Object* ret(Object *frame) {
  Object *stack        = get(frame, Stack);
  Object *ret_value    = pop(stack);
  Object *return_frame = get(frame, Return);
  if ( exists(return_frame) ) {
    Object *return_stack = get(return_frame, Stack);
    push(return_stack, ret_value);
  }
  return return_frame;
}

void interp(Object *frame) {
  Fixnum pc     = 0;
  Object *instr = 0;
  Object *new_frame = 0;

  while(frame != 0) {

    pc    = get_fixnum(frame, Pc);
    instr = get_code(frame, pc);

    if ( instr == Push ) {
      push(get(frame, Stack), get_code(frame, pc+1));
      pc += 2;
      set_fixnum(frame, Pc, pc);
      continue;
    }

    if ( instr == PushBlock ) {
      Object *stack = get(frame, Stack);
      Object *block = get_code(frame, pc+1);
      set(block, "lexical_parent", get(frame, Local));
      push(stack, block);
      pc += 2;
      set_fixnum(frame, Pc, pc);
      continue;
    }

    if ( instr == Arg ) {
      set(get(frame, Local), get_code(frame, pc+1), pop(get(get(frame, Return), Stack)));
      pc += 2;
      set_fixnum(frame, Pc, pc);
      continue;
    }

    if ( instr == Self ) {
      push(get(frame, Stack), get(frame, Local));
      pc += 1;
      set_fixnum(frame, Pc, pc);
      continue;
    }

    if ( instr == Send ) {
      new_frame = send(frame, get_code(frame, pc+1));
      if ( is_exception(new_frame) ) {
        frame = new_exception_frame(frame, new_frame);
      }
      else if ( new_frame == frame ) {
        pc += 2;
        set_fixnum(frame, Pc, pc);
      }
      else {
        pc += 2;
        set_fixnum(frame, Pc, pc);
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
      }
      else {
        pc += 2;
      }
      set_fixnum(frame, Pc, pc);
      continue;
    }

    if ( instr == JmpNotTrue ) {
      if ( pop(get(frame, Stack)) != True ) {
        pc = get_code_fixnum(frame, pc+1);
      }
      else {
        pc += 2;
      }
      set_fixnum(frame, Pc, pc);
      continue;
    }

    if ( instr == Return ) {
      frame = ret(frame);
      continue;
    }

    if ( instr == Term ) {
      return;
    }

    abort();
  }
}

