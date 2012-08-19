#include "code.h"
#include "symbols.h"
#include "array.h"
#include "native_call.h"

void code_native_call(cxt_t* cxt, fixnum code, native_call call) {
  array_push(cxt, code, SYM_PUSH);
  array_push(cxt, code, new_native_call(cxt, call));
  array_push(cxt, code, SYM_NATIVE_CALL);
};

void code_arg(cxt_t *cxt, fixnum code, fixnum slot) {
  array_push(cxt, code, SYM_ARG);
  array_push(cxt, code, slot);
}

void code_fget(cxt_t *cxt, fixnum code, fixnum slot) {
  array_push(cxt, code, SYM_FGET);
  array_push(cxt, code, slot);
}

void code_fset(cxt_t *cxt, fixnum code, fixnum slot) {
  array_push(cxt, code, SYM_FSET);
  array_push(cxt, code, slot);
}

void code_get(cxt_t *cxt, fixnum code) {
  array_push(cxt, code, SYM_GET);
}

void code_retnz(cxt_t *cxt, fixnum code) {
  array_push(cxt, code, SYM_RETNZ);
}

void code_ret(cxt_t *cxt, fixnum code) {
  array_push(cxt, code, SYM_RET);
}

void code_retz(cxt_t *cxt, fixnum code) {
  array_push(cxt, code, SYM_RETZ);
}

fixnum code_jmp(cxt_t *cxt, fixnum code, fixnum pos) {
  array_push(cxt, code, SYM_JMP);
  fixnum label = array_length(cxt, code);
  array_push(cxt, code, pos);
  return label;
}

fixnum code_jmpz(cxt_t *cxt, fixnum code, fixnum pos) {
  array_push(cxt, code, SYM_JMPZ);
  fixnum label = array_length(cxt, code);
  array_push(cxt, code, pos);
  return label;
}

fixnum code_jmpnz(cxt_t *cxt, fixnum code, fixnum pos) {
  array_push(cxt, code, SYM_JMPNZ);
  fixnum label = array_length(cxt, code);
  array_push(cxt, code, pos);
  return label;
}

void code_sget(cxt_t *cxt, fixnum code, fixnum slot) {
  array_push(cxt, code, SYM_SGET);
  array_push(cxt, code, slot);
}

void code_call(cxt_t *cxt, fixnum code) {
  array_push(cxt, code, SYM_CALL);
}

void code_push(cxt_t *cxt, fixnum code, fixnum value) {
  array_push(cxt, code, SYM_PUSH);
  array_push(cxt, code, value);
}

void code_term(cxt_t *cxt, fixnum code) {
  array_push(cxt, code, SYM_TERM);
}

fixnum code_send(cxt_t *cxt) {
  fixnum code = new_array(cxt);
  code_arg(cxt, code, SYM_SLOT);
  code_arg(cxt, code, SYM_TARGET);
  fixnum loop = array_length(cxt, code);
  code_fget(cxt, code, SYM_TARGET);
  code_fget(cxt, code, SYM_SLOT);
  code_get(cxt, code);
  code_retnz(cxt, code);
  code_fget(cxt,  code, SYM_TARGET);
  code_sget(cxt,  code, SSYM_PARENT);
  code_retz(cxt, code);
  code_fset(cxt, code, SYM_TARGET);
  code_jmp(cxt,  code, loop);
  return code;
}

void set_label(cxt_t *cxt, fixnum code, fixnum else_jmp) {
  array_set_at(cxt, code, else_jmp, array_length(cxt, code));
}

fixnum code_if_else(cxt_t *cxt) {
  fixnum code = new_array(cxt);
  code_arg(cxt, code, SYM_COND);
  code_arg(cxt, code, SYM_TRUE_BLOCK);
  code_arg(cxt, code, SYM_FALSE_BLOCK);
  code_fget(cxt, code, SYM_COND);
  code_call(cxt, code);
  fixnum else_jmp = code_jmpz(cxt, code, 0);
  code_fget(cxt, code, SYM_TRUE_BLOCK);
  code_call(cxt, code);
  code_ret(cxt, code);
  set_label(cxt, code, else_jmp);
  code_fget(cxt, code, SYM_FALSE_BLOCK);
  code_call(cxt, code);
  code_ret(cxt, code);
  return code;
};

fixnum code_while(cxt_t *cxt) {
  fixnum code = new_array(cxt);
  code_arg(cxt, code, SYM_COND);
  code_arg(cxt, code, SYM_BLOCK);
  fixnum loop = array_length(cxt, code);
  code_fget(cxt, code, SYM_COND);
  code_call(cxt, code);
  code_retz(cxt, code);
  code_fget(cxt, code, SYM_BLOCK);
  code_call(cxt, code);
  code_jmp(cxt, code, loop);
  return code;
};



