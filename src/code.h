#ifndef CODE_H
#define CODE_H

#include "rjl.h"
#include "native_call.h"

void code_native_call(cxt_t* cxt, fixnum code, native_call call);
void code_arg(cxt_t *cxt, fixnum code, fixnum slot);
void code_fget(cxt_t *cxt, fixnum code, fixnum slot);
void code_fset(cxt_t *cxt, fixnum code, fixnum slot);
void code_get(cxt_t *cxt, fixnum code);
void code_retnz(cxt_t *cxt, fixnum code);
void code_ret(cxt_t *cxt, fixnum code);
void code_retz(cxt_t *cxt, fixnum code);
fixnum code_jmp(cxt_t *cxt, fixnum code, fixnum pos);
fixnum code_jmpz(cxt_t *cxt, fixnum code, fixnum pos);
fixnum code_jmpnz(cxt_t *cxt, fixnum code, fixnum pos);
void code_sget(cxt_t *cxt, fixnum code, fixnum slot);
void code_call(cxt_t *cxt, fixnum code);
void code_push(cxt_t *cxt, fixnum code, fixnum value);
void code_term(cxt_t *cxt, fixnum code);
fixnum code_send(cxt_t *cxt);
void set_label(cxt_t *cxt, fixnum code, fixnum else_jmp);
fixnum code_if_else(cxt_t *cxt);
fixnum code_while(cxt_t *cxt);


#endif
