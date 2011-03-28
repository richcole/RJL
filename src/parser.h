
Object* parse_block_expr(Object *pc);
Object* parse_group_expr(Object *pc);
Object* parse_object_expr(Object *pc);
Object* parse_expr(Object *pc);

Object *curr(Object *pc) {
  return get_at(
    get(pc, "tokens"), 
    get_fixnum(pc, "index")
  );
}

void advance(Object *pc) {
  Fixnum index = get_fixnum(pc, "index");
  if ( index+1 < array_length(get(pc, "tokens")) ) {
    set_fixnum(pc, "index", index+1);
  }
};

Object *mark(Object *pc) {
  return get(pc, "index");
}

void restore(Object *pc, Object *mark) {
  set(pc, "index", mark);
}

Fixnum have(Object *pc, Object *token_type) {
  Object *tok = curr(pc);
  if ( get(tok, "type") == token_type ) {
    return 1;
  }
  else {
    return 0;
  }
}

Fixnum have(Object *pc, char const* token_type) {
  return have(pc, sym(token_type));
};

Object *new_parse_error(char const* str, Object *type, Object *curr) {
  Object *error = new_object();
  set(error, "expected_type", type);
  set(error, "message", new_string(str));
  set(error, "found", curr);
  return error;
}

Fixnum mustbe(Object *pc, Object *token_type) {
  if ( have(pc, token_type) ) {
    advance(pc);
    return 1;
  }
  else {
    push_slot(pc, "errors", new_parse_error("Expected token type", token_type, curr(pc)));
    advance(pc);
    return 0;
  }
}

Fixnum mustbe(Object *pc, char const* token_type) {
  return mustbe(pc, sym(token_type));
};

Fixnum have_set(Object *pc, Object *set_name) {
  Object *s = get(get(pc, set_name), get(curr(pc), "type"));
  if ( s == Undefined ) {
    return 0;
  }
  else {
    return 1;
  }
}

Fixnum have_set(Object *pc, char const* set_name) {
  return have_set(pc, sym(set_name));
}

Object* parse_expr_list(Object *pc) {
  Object *expr = new_object();
  set(expr, "type", "expr_list");
  while (have_set(pc, "begin_expr")) {
    push_slot(expr, "exprs", parse_expr(pc));
  }
  return expr;
}

Object* parse_expr(Object *pc) {
  Object *expr = new_object();
  if ( have(pc, "ident") ) {
    set(expr, "type", "send_expr");
    set(expr, "target", curr(pc));
    advance(pc);
  }
  else if ( have(pc, "arg_ident") ) {
    set(expr, "type", "send_arg_expr");
    while ( have(pc, "arg_ident") ) {
      push_slot(expr, "arg_names", curr(pc));
      advance(pc);
      push_slot(expr, "args", parse_expr(pc));
    }
  }
  else if ( have(pc, "string") ) {
    set(expr, "type", "string_literal");
    set(expr, "value", get(curr(pc), "value"));
    advance(pc);
  }
  else if ( have(pc, "block_open") ) {
    expr = parse_block_expr(pc);
  }
  else if ( have(pc, "group_open") ) {
    expr = parse_group_expr(pc);
  }
  else if ( have(pc, "object_open") ) {
    expr = parse_object_expr(pc);
  }
  else if ( have(pc, "operator") ) {
    set(expr, "type", "operator_expr");
    set(expr, "op",   curr(pc));
    advance(pc);
    set(expr, "arg",  parse_expr(pc));
  }
  else if ( have(pc, "number_literal") ) {
    set(expr, "type", "number_literal");
    set(expr, "value", string_to_boxed_number(get(curr(pc), "value")));
    advance(pc);
  }
  else if ( have(pc, "symbol") ) {
    set(expr, "type", "symbol_literal");
    set(expr, "value", sym(get(curr(pc), "value")));
    advance(pc);
  }
  return expr;
}

Object* parse_stmt(Object *pc) {
  Object *stmt = 0;
  if ( have_set(pc, "begin_expr") ) {
    stmt = parse_expr_list(pc);
  }
  else if ( have(pc, "if") ) {
    mustbe(pc, "if");
    stmt = new_object();
    set(stmt, "type", "if_stmt");
    set(stmt, "cond", parse_group_expr(pc));
    set(stmt, "true_block", parse_block_expr(pc));
    if ( have(pc, "else") ) {
      mustbe(pc, "else");
      set(stmt, "false_block", parse_block_expr(pc));
    }
  }
  else if ( have(pc, "while") ) {
    mustbe(pc, "while");
    stmt = new_object();
    set(stmt, "type", "while_stmt");
    set(stmt, "cond", parse_group_expr(pc));
    set(stmt, "while_block", parse_block_expr(pc));
  }
  else if ( have(pc, "return") ) {
    stmt = new_object();
    set(stmt, "type", "return_stmt");
    advance(pc);
    set(stmt, "expr", parse_expr_list(pc));
  }
  mustbe(pc, "semi");
  return stmt;
}

Object* parse_block_expr(Object *pc) {
  Object *block = new_object();
  set(block, "type", "block_expr");

  if ( ! mustbe(pc, BlockOpen) ) return block;

  Object *begin_mark = mark(pc);

  while ( have(pc, Ident) ) {
    push_slot(block, "args", curr(pc));
    advance(pc);
  }

  if ( ! have(pc, "pipe") ) { 
    set(block, "args", Undefined);
    restore(pc, begin_mark);
  }
  else {
    advance(pc);
  }

  while ( ! have_set(pc, "end_block") ) {
    push_slot(block, "stmts", parse_stmt(pc));
  }
  mustbe(pc, "block_close");
  return block;
}

Object* parse_object_expr(Object *pc) {
  Object *object = new_object();
  set(object, "type", "object_expr");

  if ( ! mustbe(pc, ObjectOpen) ) return object;

  while ( ! have_set(pc, "end_object") ) {
    push_slot(object, "stmts", parse_stmt(pc));
  }
  mustbe(pc, "object_close");
  return object;
}

Object* parse_group_expr(Object *pc) {
  Object *group = new_object();
  set(group, "type", "group");
  if ( ! mustbe(pc, GroupOpen) ) return group;
  set(group, "expr_list", parse_expr_list(pc));
  mustbe(pc, "group_close");
  return group;
}

void create_sets(Object *pc) {
  Object *end_block = new_object();
  set(pc, "end_block", end_block);
  set(end_block, "block_close", "true");
  set(end_block, "eof", "true");

  Object *end_object = new_object();
  set(pc, "end_object", end_object);
  set(end_object, "object_close", "true");
  set(end_object, "eof", "true");

  Object *begin_expr = new_object();
  set(pc, "begin_expr", begin_expr);
  set(begin_expr, "ident", "true");
  set(begin_expr, "arg_ident", "true");
  set(begin_expr, "string", "true");
  set(begin_expr, "operator", "true");
  set(begin_expr, "number_literal", "true");
  set(begin_expr, "symbol_literal", "true");
  set(begin_expr, "group_open", "true");
};

Object *parse(Object *sc) {
  Object *pc = new_object();
  set(pc, "scan_context", sc);
  set(pc, "tokens", get(sc, "tokens"));
  set(pc, "index", object(0));
  create_sets(pc);
  set(pc, "block", parse_block_expr(pc));
  return pc;
}

Object* native_parser_tokenize(Object *frame, Object *self) {
  Object *scan_context = pop(get(frame, Stack));
  push(get(frame, Stack), parse(scan_context));
  return frame;
}

void init_parser_symbols() {
}

void init_parser_sys(Object *sys) {
  Object *parser_object = new_object();
  set(sys, "Parser", parser_object);
  set(parser_object, "parse:", new_func(native_parser_tokenize));
}
