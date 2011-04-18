#include "code_generator.h"
#include "std.h"
#include "foreach.h"
#include "code.h"
#include "abort.h"
#include "block.h"

void code_gen_args(Object *cxt, Object *pc, Object *code, Object *block) {
  Object *args = get(cxt, block, sym(cxt, "args"));
  FOR_EACH_ARRAY_REV(cxt, i, arg, args) {
    code_arg(cxt, code, sym(cxt, get(cxt, arg, sym(cxt, "value"))));
  }
}

Fixnum has_type(Object *cxt, Object *o, char const* type) {
  return get(cxt, o, sym(cxt, "type")) == sym(cxt, type);
};

void code_depedent_send(Object *cxt, Object *code, Fixnum i, Object *slot) {
  if ( i == 0 ) {
    code_self_send(cxt, code, sym(cxt, slot));
  }
  else {
    code_send(cxt, code, sym(cxt, slot));
  }
}

Object *join_char_arrays(Object *cxt, Object *strs) {
  Fixnum total_length = 0;
  {
    FOR_EACH_ARRAY(cxt, i, str, strs) {
      total_length += char_array_length(cxt, get(cxt, str, "value"));
    }
  }
  Object *result = new_char_array(cxt, total_length);
  {
    FOR_EACH_ARRAY(cxt, i, str, strs) {
      char_array_append(cxt, result, get(cxt, str, "value"));
    }
  }
  return result;
}

void code_gen_expr(Object *cxt, Object* pc, Object* code, Object* block, Object *expr) {
  if ( has_type(cxt, expr, "char_array_literal") ) {
    code_push(cxt, code, get(cxt, expr, "value"));
  }
  else if ( has_type(cxt, expr, "block_expr") ) {
    code_push_frame(cxt, code);
    code_push_block(cxt, code, code_gen_block(cxt, pc, expr, get_false(cxt)));
  }
  else if ( has_type(cxt, expr, "group") ) {
    code_gen_group(cxt, pc, code, block, expr);
  }
  else if ( has_type(cxt, expr, "number_literal") ) {
    code_push(cxt, code, get(cxt, expr, "value"));
  }
  else if ( has_type(cxt, expr, "symbol_literal") ) {
    code_push(cxt, code, get(cxt, expr, "value"));
  }
  else if ( has_type(cxt, expr, "send_expr") ) {
    code_self_send(cxt, code, sym(cxt, get(cxt, get(cxt, expr, "target"), "value")));
  }
  else if ( has_type(cxt, expr, "object_expr") ) {
    code_push_frame(cxt, code);
    code_push_block(cxt, code, code_gen_block(cxt, pc, expr, get_false(cxt)));
    code_self_send(cxt, code, "Object");
    code_send(cxt, code, "new:");
  }
}

void code_gen_expr_list_stmt(
  Object *cxt, Object* pc, Object* code, Object* block, Object *expr_list
) {
  // first evaluate the arguments in reverse order
  {
    FOR_EACH_ARRAY_REV(cxt, i, expr, expr_list) {
      if ( has_type(cxt, expr, "send_arg_expr") ) {
        Object *args = get(cxt, expr, "args");
        FOR_EACH_ARRAY(cxt, j, arg, args) {
          code_gen_expr(cxt, pc, code, block, arg);
        }
      }
      if ( has_type(cxt, expr, "operator_expr") ) {
        code_gen_expr(cxt, pc, code, block, get(cxt, expr, "arg"));
      }
    }
  }

  // next evaluate the sends in order
  {
    FOR_EACH_ARRAY(cxt, i, expr, expr_list) {
      if ( has_type(cxt, expr, "send_expr") ) {
        code_depedent_send(cxt, code, i, get(cxt, get(cxt, expr, sym(cxt, "target")), "value"));
      }
      else if ( has_type(cxt, expr, "send_arg_expr") ) {
        code_depedent_send(cxt, code, i, join_char_arrays(cxt, get(cxt, expr, "arg_names")));
      }
      else if ( has_type(cxt, expr, "operator_expr") ) {
        code_depedent_send(cxt, code, i, char_array_concat(cxt, get(cxt, get(cxt, expr, "op"), "value"), sym(cxt, ":")));
      }
      else if ( has_type(cxt, expr, "group") ) {
        code_gen_group(cxt, pc, code, block, expr);
      }
      else if ( has_type(cxt, expr, "number_literal") ) {
        code_push(cxt, code, get(cxt, expr, "value"));
      }
      else {
        abort();
      }
    }
  }
}

void code_gen_local_block(Object *cxt, Object *pc, Object *code, Object *stmt, Object *is_local_block) {
  code_push_frame(cxt, code);
  if ( is_true(cxt, is_local_block) ) {
    code_send(cxt, code, "non_local_return");
  }
  else {
    code_send(cxt, code, "return");
  }
  code_push_block(cxt, code, code_gen_block(cxt, pc, stmt, get_true(cxt)));
};

void code_gen_stmt(Object *cxt, Object* pc, Object* code, Object* block, Object *stmt, Object *is_local_block) {
  if ( has_type(cxt, stmt, "expr_list") ) {
    Object *exprs = get(cxt, stmt, sym(cxt, "exprs"));
    code_gen_expr_list_stmt(cxt, pc, code, block, exprs);
  }
  else if ( has_type(cxt, stmt, "if_stmt") ) {
    code_gen_local_block(cxt, pc, code, get(cxt, stmt, "true_block"), is_local_block);
    code_gen_local_block(cxt, pc, code, get(cxt, stmt, "false_block"), is_local_block);
    code_gen_group(cxt, pc, code, block, get(cxt, stmt, "cond"));
    code_self_send(cxt, code, sym(cxt, "if:else:"));
  }
  else if ( has_type(cxt, stmt, "while_stmt") ) {
    Fixnum offset = array_length(cxt, code);
    code_gen_group(cxt, pc, code, block, get(cxt, stmt, "cond"));
    Fixnum jmp_offset = array_length(cxt, code)+1;
    push(cxt, code, "jmp_not_true");
    push(cxt, code, new_boxed_int(cxt, array_length(cxt, code)));
    code_gen_local_block(cxt, pc, code, get(cxt, stmt, "while_block"), is_local_block);
    code_send(cxt, code, sym(cxt, "call"));
    push(cxt, code, "jmp");
    push(cxt, code, new_boxed_int(cxt, offset));
    set_at(cxt, code, jmp_offset, new_boxed_int(cxt, array_length(cxt, code)));
  }
  else if ( has_type(cxt, stmt, "return_stmt") ) {
    Object *exprs =  get(cxt, get(cxt, stmt, "expr"), "exprs");
    code_gen_expr_list_stmt(cxt, pc, code, block, exprs);
    code_return(cxt, code);
  }
  else {
    abort();
  }
}

void code_gen_stmts(Object *cxt, Object *pc, Object *code, Object *block, Object *is_local_block) {
  Object *stmts = get(cxt, block, sym(cxt, "stmts"));
  FOR_EACH_ARRAY(cxt, i, stmt, stmts) {
    code_gen_stmt(cxt, pc, code, block, stmt, is_local_block);
  }
}

Object* code_gen_block(Object *cxt, Object *pc, Object *block, 
                       Object *is_local_block) 
{
  Object *code = new_block(cxt);
  code_gen_args(cxt, pc, code, block);
  code_gen_stmts(cxt, pc, code, block, is_local_block);
  code_local_return(cxt, code);
  return code;
}

void code_gen_group(Object *cxt, Object *pc, Object *code, Object *block, Object *group) {
  Object *exprs = get(cxt, get(cxt, group, "expr_list"), "exprs");
  code_gen_expr_list_stmt(cxt, pc, code, block, exprs);
}

Object* code_generator_generate(Object *cxt, Object *pc) {
  Object *block = get(cxt, pc, sym(cxt, "block"));
  set(cxt, pc, "code:", code_gen_block(cxt, pc, block, get_false(cxt)));
  return pc;
}

Object* native_code_generator_generate(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *parse_context = pop(cxt, stack);
  push(cxt, stack, code_generator_generate(cxt, parse_context));
  return frame;
}

void init_code_generator_sys(Object *cxt) {
  Object *code_generator = context_get(cxt, "CodeGenerator");
  set(cxt, code_generator, "generate:", new_func(cxt, native_code_generator_generate));
}
