Object* code_generator_generate(Object *parse_context) {
  return parse_context;
}

Object* native_code_generator_generate(Object *frame, Object *self) {
  Object *stack = get(frame, Stack);
  Object *parse_context = pop(stack);
  push(stack, code_generator_generate(parse_context));
  return frame;
}

void init_code_generator_symbols() {
}

void init_code_generator_sys(Object *sys) {
  Object *code_generator_object = new_object();
  set(sys, sym("CodeGenerator"), code_generator_object);
  set(code_generator_object, sym("generate:"), 
      new_func(native_code_generator_generate));
}
