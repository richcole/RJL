void code_push(Object *code, Object *val) {
  push(code, Push);
  push(code, val);
}

void code_push_block(Object *code, Object *val) {
  push(code, PushBlock);
  push(code, val);
}

void code_send(Object *code, Object *slot) {
  push(code, Send);
  push(code, slot);
}

void code_send(Object *code, char const* slot) {
  code_send(code, sym(slot));
}

void code_self_send(Object *code, Object *slot) {
  push(code, Self);
  push(code, Send);
  push(code, slot);
}

void code_self_send(Object *code, char const* slot) {
  code_self_send(code, sym(slot));
}

void code_self(Object *code) {
  push(code, Self);
}

void code_return(Object *code) {
  push(code, Return);
}

void code_local_return(Object *code) {
  push(code, LocalReturn);
}

void code_term(Object *code) {
  push(code, Term);
}

void code_arg(Object *code, Object *arg_name) {
  push(code, Arg);
  push(code, arg_name);
}

void code_arg(Object *code, char const* arg_name) {
  return code_arg(code, sym(arg_name));
}
