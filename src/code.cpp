#include "code.h"
#include "array.h"
#include "sym.h"

void code_push(Object *cxt, Object *code, Object *val) {
  push(cxt, code, "push");
  push(cxt, code, val);
}

void code_push_frame(Object *cxt, Object *code) {
  push(cxt, code, "push_frame");
}

void code_push_block(Object *cxt, Object *code, Object *block) {
  push(cxt, code, "push_block");
  push(cxt, code, block);
}

void code_send(Object *cxt, Object *code, Object *slot) {
  push(cxt, code, "send");
  push(cxt, code, slot);
}

void code_send(Object *cxt, Object *code, char const* slot) {
  code_send(cxt, code, sym(cxt, slot));
}

void code_self_send(Object *cxt, Object *code, Object *slot) {
  push(cxt, code, "self");
  push(cxt, code, "send");
  push(cxt, code, slot);
}

void code_self_send(Object *cxt, Object *code, char const* slot) {
  code_self_send(cxt, code, sym(cxt, slot));
}

void code_self(Object *cxt, Object *code) {
  push(cxt, code, "self");
}

void code_return(Object *cxt, Object *code) {
  push(cxt, code, "return");
}

void code_local_return(Object *cxt, Object *code) {
  push(cxt, code, "local_return");
}

void code_term(Object *cxt, Object *code) {
  push(cxt, code, "term");
}

void code_arg(Object *cxt, Object *code, Object *arg_name) {
  push(cxt, code, "arg");
  push(cxt, code, arg_name);
}

void code_arg(Object *cxt, Object *code, char const* arg_name) {
  return code_arg(cxt, code, sym(cxt, arg_name));
}
