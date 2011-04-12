Object* code_gen_block(Object *pc, Object *block);

void code_gen_group(Object *pc, Object *code, Object *block, Object *group);

void code_gen_args(Object *pc, Object *code, Object *block) {
  Object *args = get(block, sym("args"));
  FOR_EACH_ARRAY_REV(i, arg, args) {
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
  else if ( has_type(expr, "block_expr") ) {
    code_push_block(code, code_gen_block(pc, expr));
  }
  else if ( has_type(expr, "group") ) {
    code_gen_group(pc, code, block, expr);
  }
  else if ( has_type(expr, "number_literal") ) {
    code_push(code, get(expr, "value"));
  }
  else if ( has_type(expr, "symbol_literal") ) {
    code_push(code, get(expr, "value"));
  }
  else if ( has_type(expr, "send_expr") ) {
    code_self_send(code, sym(get(get(expr, "target"), "value")));
  }
  else if ( has_type(expr, "object_expr") ) {
    code_push_block(code, code_gen_block(pc, expr));
    code_self_send(code, "Object");
    code_send(code, "new:");
  }
}

void code_gen_expr_list_stmt(
  Object* pc, Object* code, Object* block, Object *expr_list
) {
  // first evaluate the arguments in reverse order
  {
    FOR_EACH_ARRAY_REV(i, expr, expr_list) {
      if ( has_type(expr, "send_arg_expr") ) {
        Object *args = get(expr, "args");
        FOR_EACH_ARRAY(j, arg, args) {
          code_gen_expr(pc, code, block, arg);
        }
      }
      if ( has_type(expr, "operator_expr") ) {
        code_gen_expr(pc, code, block, get(expr, "arg"));
      }
    }
  }

  // next evaluate the sends in order
  {
    FOR_EACH_ARRAY(i, expr, expr_list) {
      if ( has_type(expr, "send_expr") ) {
        code_depedent_send(code, i, get(get(expr, sym("target")), "value"));
      }
      else if ( has_type(expr, "send_arg_expr") ) {
        code_depedent_send(code, i, join_strings(get(expr, "arg_names")));
      }
      else if ( has_type(expr, "operator_expr") ) {
        code_depedent_send(code, i, string_concat(get(get(expr, "op"), "value"), sym(":")));
      }
      else if ( has_type(expr, "group") ) {
        code_gen_group(pc, code, block, expr);
      }
      else if ( has_type(expr, "number_literal") ) {
        code_push(code, get(expr, "value"));
      }
      else {
        abort();
      }
    }
  }
}

void code_gen_stmt(Object* pc, Object* code, Object* block, Object *stmt) {
  if ( has_type(stmt, "expr_list") ) {
    Object *exprs = get(stmt, sym("exprs"));
    code_gen_expr_list_stmt(pc, code, block, exprs);
  }
  else if ( has_type(stmt, "if_stmt") ) {
    code_push_block(code, code_gen_block(pc, get(stmt, "true_block")));
    code_push_block(code, code_gen_block(pc, get(stmt, "false_block")));
    code_gen_group(pc, code, block, get(stmt, "cond"));
    code_self_send(code, sym("if:else:"));
  }
  else if ( has_type(stmt, "while_stmt") ) {
    Fixnum offset = array_length(code);
    code_gen_group(pc, code, block, get(stmt, "cond"));
    Fixnum jmp_offset = array_length(code)+1;
    push(code, JmpNotTrue);
    push(code, object(array_length(code)));
    code_push_block(code, code_gen_block(pc, get(stmt, "while_block")));
    code_send(code, sym("call"));
    push(code, Jmp);
    push(code, object(offset));
    set_at(code, jmp_offset, object(array_length(code)));
  }
  else if ( has_type(stmt, "return_stmt") ) {
    Object *exprs =  get(get(stmt, "expr"), "exprs");
    code_gen_expr_list_stmt(pc, code, block, exprs);
  }
  else {
    abort();
  }
}

void code_gen_stmts(Object *pc, Object *code, Object *block) {
  Object *stmts = get(block, sym("stmts"));
  FOR_EACH_ARRAY(i, stmt, stmts) {
    code_gen_stmt(pc, code, block, stmt);
  }
}

Object* code_gen_block(Object *pc, Object *block) {
  Object *code = new_block();
  code_gen_args(pc, code, block);
  code_gen_stmts(pc, code, block);
  code_return(code);
  return code;
}

void code_gen_group(Object *pc, Object *code, Object *block, Object *group) {
  Object *exprs = get(get(group, "expr_list"), "exprs");
  code_gen_expr_list_stmt(pc, code, block, exprs);
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
