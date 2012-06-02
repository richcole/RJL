#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "object.h"

Object* code_gen_block(Object *cxt, Object *pc, Object *block, Object *catch_stmts);
void code_gen_push_block(Object *cxt, Object *pc, Object *code, Object *block, Object *catch_stmt, Object *finally_stmt);
void code_gen_group(Object *cxt, Object *pc, Object *code, Object *block, Object *group);
void init_code_generator_sys(Object *cxt);

#endif

