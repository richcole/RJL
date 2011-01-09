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

