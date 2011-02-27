void code_gen_args(Object *pc, Object *code, Object *block) {
  Object *args = get(block, sym("args"));
  FOR_EACH_ARRAY(i, arg, args) {
    code_arg(code, sym(get(arg, sym("value"))));
  }
}

Fixnum has_type(Object *o, char const* type) {
  return get(o, sym("type")) == sym(type);
};

void code_depedent_send(Object *code, Fixnum i, Object *slot) {
  if ( i == 0 ) {
    code_self_send(code, sym(slot));
  }
  else {
    code_send(code, sym(slot));
  }
}

Object *join_strings(Object *strs) {
  Fixnum total_length = 0;
  {
    FOR_EACH_ARRAY(i, str, strs) {
      total_length += string_length(get(str, "value"));
    }
  }
  Object *result = new_string(total_length);
  {
    FOR_EACH_ARRAY(i, str, strs) {
      string_append(result, get(str, "value"));
    }
  }
  return result;
}

void code_gen_expr(Object* pc, Object* code, Object* block, Object *expr) {
  if ( has_type(expr, "string_literal") ) {
    code_push(code, get(expr, "value"));
  }
}

void code_gen_expr_list_stmt(
  Object* pc, Object* code, Object* block, Object *expr_list
) {
  FOR_EACH_ARRAY(i, expr, expr_list) {
    if ( i+1 < array_length(expr_list) ) {
      Object *next_expr = get_at(expr_list, i+1);
      if ( has_type(next_expr, "send_arg_expr") ) {
        Object *args = get(next_expr, "args");
        FOR_EACH_ARRAY(j, arg, args) {
          code_gen_expr(pc, code, block, arg);
        }
      }
    }
    if ( has_type(expr, "send_expr") ) {
      code_depedent_send(code, i, get(get(expr, sym("target")), "value"));
    }
    else if ( has_type(expr, "send_arg_expr") ) {
      code_depedent_send(code, i, join_strings(get(expr, "arg_names")));
    }
  }
}

void code_gen_stmt(Object* pc, Object* code, Object* block, Object *stmt) {
  if ( has_type(stmt, "expr_list") ) {
    code_gen_expr_list_stmt(pc, code, block, get(stmt, sym("exprs")));
  }
}

void code_gen_stmts(Object *pc, Object *code, Object *block) {
  Object *stmts = get(block, sym("stmts"));
  FOR_EACH_ARRAY(i, stmt, stmts) {
    code_gen_stmt(pc, code, block, stmt);
  }
}

Object* code_gen_block(Object *pc, Object *block) {
  Object *code = new_array();
  set(code, "is_block", True);
  code_gen_args(pc, code, block);
  code_gen_stmts(pc, code, block);
  code_return(code);
  return code;
}

Object* code_generator_generate(Object *pc) {
  Object *block = get(pc, sym("block"));
  set(pc, sym("code:"), code_gen_block(pc, block));
  return pc;
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
