
Object *curr(Object *pc) {
  return get_at(
    get(pc, sym("tokens")), 
    get_fixnum(pc, sym("index"))
  );
}

void advance(Object *pc) {
  set_fixnum(pc, sym("index"), get_fixnum(pc, sym("index"))+1);
};

Fixnum have(Object *pc, Object *token_type) {
  Object *tok = curr(pc);
  if ( get(tok, sym("type")) == token_type ) {
    return 1;
  }
  else {
    return 0;
  }
}

Object *new_parse_error(char const* str, Object *type, Object *curr) {
  Object *error = new_object();
  set(error, sym("expected_type"), type);
  set(error, sym("message"), new_string(str));
  set(error, sym("found"), curr);
  return error;
}

Fixnum mustbe(Object *pc, Object *token_type) {
  if ( have(pc, token_type) ) {
    advance(pc);
    return 1;
  }
  else {
    push_slot(pc, sym("errors"), new_parse_error("Expected token type", token_type, curr(pc)));
    return 0;
  }
}

Fixnum have_set(Object *pc, Object *set_name) {
  Object *s = get(get(pc, set_name), get(curr(pc), sym("type")));
  if ( s == Undefined ) {
    return 0;
  }
  else {
    return 1;
  }
}

Object* parse_expr(Object *pc);

Object* parse_expr_list(Object *pc) {
  Object *expr = new_object();
  set(expr, sym("type"), sym("expr_list"));
  while (have_set(pc, sym("begin_expr"))) {
    push_slot(expr, sym("exprs"), parse_expr(pc));
  }
  return expr;
}

Object* parse_expr(Object *pc) {
  Object *expr = new_object();
  if ( have(pc, sym("ident")) ) {
    set(expr, sym("type"), sym("send_expr"));
    set(expr, sym("target"), curr(pc));
    advance(pc);
  }
  else if ( have(pc, sym("arg_ident")) ) {
    set(expr, sym("type"), sym("send_arg_expr"));
    while ( have(pc, sym("arg_ident")) ) {
      push_slot(expr, sym("arg_names"), curr(pc));
      advance(pc);
      push_slot(expr, sym("args"), parse_expr(pc));
    }
  }
  else if ( have(pc, sym("string")) ) {
    set(expr, sym("type"), sym("string_literal"));
    set(expr, sym("value"), get(curr(pc), sym("value")));
    advance(pc);
  }
  return expr;
}

Object* parse_stmt(Object *pc) {
  Object *stmt = 0;
  if ( have_set(pc, sym("begin_expr")) ) {
    stmt = parse_expr_list(pc);
    mustbe(pc, sym("semi"));
  }
  return stmt;
}

Object* parse_block(Object *pc) {
  Object *block = new_object();
  if ( ! mustbe(pc, BlockOpen) ) return block;
  while ( have(pc, Ident) ) {
    push_slot(block, sym("args"), curr(pc));
    advance(pc);
  }
  if ( ! mustbe(pc, Pipe) ) return block;
  if ( ! have_set(pc, sym("end_block")) ) {
    push_slot(block, sym("stmts"), parse_stmt(pc));
  }
  mustbe(pc, sym("block_close"));
  return block;
}

void create_sets(Object *pc) {
  Object *end_block = new_object();
  set(pc, sym("end_block"), end_block);
  set(end_block, sym("block_close"), sym("true"));
  set(end_block, sym("eof"), sym("true"));

  Object *begin_expr = new_object();
  set(pc, sym("begin_expr"), begin_expr);
  set(begin_expr, sym("ident"), sym("true"));
  set(begin_expr, sym("arg_ident"), sym("true"));
  set(begin_expr, sym("string"), sym("true"));
};

Object *parse(Object *sc) {
  Object *pc = new_object();
  set(pc, sym("scan_context"), sc);
  set(pc, sym("tokens"), get(sc, sym("tokens")));
  set(pc, sym("index"), 0);
  create_sets(pc);
  set(pc, sym("block"), parse_block(pc));
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
  set(sys, sym("Parser"), parser_object);
  set(parser_object, sym("parse:"), new_func(native_parser_tokenize));
}
