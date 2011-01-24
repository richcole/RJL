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
  else {
    push(stack, value);
    return frame;
  }
};

Object* ret(Object *frame) {
  Object *stack        = get(frame, Stack);
  Object *ret_value    = pop(stack);
  Object *parent_frame = get(frame, Parent);
  if ( exists(parent_frame) ) {
    Object *parent_stack = get(parent_frame, Stack);
    push(parent_stack, ret_value);
  }
  return parent_frame;
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

    if ( instr == Arg ) {
      set(get(frame, Local), get_code(frame, pc+1), pop(get(get(frame, Parent), Stack)));
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
      continue;
    }

    if ( instr == Jmp ) { 
      pc += get_code_fixnum(frame, pc+1);
      set_fixnum(frame, Pc, pc);
      continue;
    }

    if ( instr == JmpZ ) {
      if ( pop(get(frame, Stack)) == 0 ) {
        pc += get_code_fixnum(frame, pc+1);
      }
      else {
        pc += 2;
      }
      set_fixnum(frame, Pc, pc);
      continue;
    }

    if ( instr == JmpNZ ) {
      if ( pop(get(frame, Stack)) == 0 ) {
        pc += get_code_fixnum(frame, pc+1);
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

