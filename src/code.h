#ifndef CODE_H
#define CODE_H

#include "object.h"

void code_push(Object *cxt, Object *code, Object *val);
void code_pop(Object *cxt, Object *code);
void code_push_block(Object *cxt, Object *code, Object *block);
void code_send(Object *cxt, Object *code, Object *slot);
void code_send(Object *cxt, Object *code, char const* slot);
void code_self_send(Object *cxt, Object *code, Object *slot);
void code_self_send(Object *cxt, Object *code, char const* slot);
void code_self(Object *cxt, Object *code);
void code_return(Object *cxt, Object *code);
void code_term(Object *cxt, Object *code);
void code_arg(Object *cxt, Object *code, Object *arg_name);
void code_arg(Object *cxt, Object *code, char const* arg_name);
Fixnum code_jmp_not_true(Object *cxt, Object *code, Fixnum jmp_location);
Fixnum code_jmp_true(Object *cxt, Object *code, Fixnum jmp_location);
Fixnum code_jmp(Object *cxt, Object *code, Fixnum jmp_location);

#endif
