void code_push(Object *code, Object *val) {
  push(code, Push);
  push(code, val);
}

void code_send(Object *code, Object *slot) {
  push(code, Send);
  push(code, slot);
}

void code_self_send(Object *code, Object *slot) {
  push(code, Self);
  push(code, Send);
  push(code, slot);
}

void code_self(Object *code) {
  push(code, Self);
}

void code_return(Object *code) {
  push(code, Return);
}

void code_term(Object *code) {
  push(code, Term);
}

void code_arg(Object *code, Object *arg_name) {
  push(code, Arg);
  push(code, arg_name);
}

