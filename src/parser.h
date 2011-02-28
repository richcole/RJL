
Object* parse_block(Object *pc);
Object* parse_group(Object *pc);
Object* parse_if_expr(Object *pc);
Object* parse_expr(Object *pc);

Object *curr(Object *pc) {
  return get_at(
    get(pc, "tokens"), 
    get_fixnum(pc, "index")
  );
}

void advance(Object *pc) {
  set_fixnum(pc, "index", get_fixnum(pc, "index")+1);
};

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

Object* parse_if_expr(Object *pc) {
  Object* expr = new_object();
  set(expr, "type", sym("if_expr"));
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
    expr = parse_block(pc);
  }
  else if ( have(pc, "group_open") ) {
    expr = parse_group(pc);
  }
  else if ( have(pc, "if") ) {
    expr = parse_if_expr(pc);
  }
  return expr;
}

Object* parse_stmt(Object *pc) {
  Object *stmt = 0;
  if ( have_set(pc, "begin_expr") ) {
    stmt = parse_expr_list(pc);
    mustbe(pc, "semi");
  }
  return stmt;
}

Object* parse_block(Object *pc) {
  Object *block = new_object();
  if ( ! mustbe(pc, BlockOpen) ) return block;
  while ( have(pc, Ident) ) {
    push_slot(block, "args", curr(pc));
    advance(pc);
  }
  if ( ! mustbe(pc, Pipe) ) return block;
  if ( ! have_set(pc, "end_block") ) {
    push_slot(block, "stmts", parse_stmt(pc));
  }
  mustbe(pc, "block_close");
  return block;
}

Object* parse_group(Object *pc) {
  Object *group = new_object();
  if ( ! mustbe(pc, GroupOpen) ) return group;
  parse_expr(pc);
  mustbe(pc, "block_close");
  return group;
}

void create_sets(Object *pc) {
  Object *end_block = new_object();
  set(pc, "end_block", end_block);
  set(end_block, "block_close", "true");
  set(end_block, "eof", "true");

  Object *begin_expr = new_object();
  set(pc, "begin_expr", begin_expr);
  set(begin_expr, "ident", "true");
  set(begin_expr, "arg_ident", "true");
  set(begin_expr, "string", "true");
};

Object *parse(Object *sc) {
  Object *pc = new_object();
  set(pc, "scan_context", sc);
  set(pc, "tokens", get(sc, "tokens"));
  set(pc, "index", object(0));
  create_sets(pc);
  set(pc, "block", parse_block(pc));
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
