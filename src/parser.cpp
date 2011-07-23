#include "parser.h"
#include "std.h"

Object* parse_block_expr(Object *cxt, Object *pc);
Object* parse_group_expr(Object *cxt, Object *pc);
Object* parse_object_expr(Object *cxt, Object *pc);
Object* parse_expr(Object *cxt, Object *pc);

Object *curr(Object* cxt, Object *pc) {
  return get_at(cxt, 
    get(cxt, pc, "tokens"), 
    get_fixnum(cxt, pc, "index")
  );
}

void advance(Object* cxt, Object *pc) {
  Fixnum index = get_fixnum(cxt, pc, "index");
  if ( index+1 < array_length(cxt, get(cxt, pc, "tokens")) ) {
    set_fixnum(cxt, pc, "index", index+1);
  }
};

Object *mark(Object* cxt, Object *pc) {
  return get(cxt, pc, "index");
}

void restore(Object* cxt, Object *pc, Object *mark) {
  set(cxt, pc, "index", mark);
}

Fixnum have(Object* cxt, Object *pc, Object *token_type) {
  Object *tok = curr(cxt, pc);
  if ( get(cxt, tok, "type") == token_type ) {
    return 1;
  }
  else {
    return 0;
  }
}

Fixnum have(Object* cxt, Object *pc, char const* token_type) {
  return have(cxt, pc, sym(cxt, token_type));
};

Object *new_parse_error(Object* cxt, char const* str, Object *type, Object *curr) {
  Object *error = new_object(cxt, "Object");
  set(cxt, error, "expected_type", type);
  set(cxt, error, "message", new_char_array(cxt, str));
  set(cxt, error, "found", curr);
  return error;
}

Fixnum mustbe(Object* cxt, Object *pc, Object *token_type) {
  if ( have(cxt, pc, token_type) ) {
    advance(cxt, pc);
    return 1;
  }
  else {
    push_slot(cxt, pc, "errors", new_parse_error(cxt, "Expected token type", token_type, curr(cxt, pc)));
    advance(cxt, pc);
    return 0;
  }
}

Fixnum mustbe(Object* cxt, Object *pc, char const* token_type) {
  return mustbe(cxt, pc, sym(cxt, token_type));
};

Fixnum have_set(Object* cxt, Object *pc, Object *set_name) {
  Object *s = get(cxt, get(cxt, pc, set_name), get(cxt, curr(cxt, pc), "type"));
  return exists(cxt, s);
}

Fixnum have_set(Object* cxt, Object *pc, char const* set_name) {
  return have_set(cxt, pc, sym(cxt, set_name));
}

Object* parse_expr_list(Object* cxt, Object *pc) {
  Object *expr = new_object(cxt, "Object");
  set(cxt, expr, "type", "expr_list");
  while (have_set(cxt, pc, "begin_expr")) {
    push_slot(cxt, expr, "exprs", parse_expr(cxt, pc));
  }
  return expr;
}

Object* parse_expr(Object* cxt, Object *pc) {
  Object *expr = new_object(cxt, "Object");
  if ( have(cxt, pc, "ident") ) {
    set(cxt, expr, "type", "send_expr");
    set(cxt, expr, "target", curr(cxt, pc));
    advance(cxt, pc);
  }
  else if ( have(cxt, pc, "arg_ident") ) {
    set(cxt, expr, "type", "send_arg_expr");
    while ( have(cxt, pc, "arg_ident") ) {
      push_slot(cxt, expr, "arg_names", curr(cxt, pc));
      advance(cxt, pc);
      push_slot(cxt, expr, "args", parse_expr(cxt, pc));
    }
  }
  else if ( have(cxt, pc, "char_array") ) {
    set(cxt, expr, "type", "char_array_literal");
    set(cxt, expr, "value", get(cxt, curr(cxt, pc), "value"));
    advance(cxt, pc);
  }
  else if ( have(cxt, pc, "block_open") ) {
    expr = parse_block_expr(cxt, pc);
  }
  else if ( have(cxt, pc, "group_open") ) {
    expr = parse_group_expr(cxt, pc);
  }
  else if ( have(cxt, pc, "object_open") ) {
    expr = parse_object_expr(cxt, pc);
  }
  else if ( have(cxt, pc, "operator") ) {
    set(cxt, expr, "type", "operator_expr");
    set(cxt, expr, "op",   curr(cxt, pc));
    advance(cxt, pc);
    set(cxt, expr, "arg",  parse_expr(cxt, pc));
  }
  else if ( have(cxt, pc, "number_literal") ) {
    set(cxt, expr, "type", "number_literal");
    set(cxt, expr, "value", char_array_to_boxed_int(cxt, get(cxt, curr(cxt, pc), "value")));
    advance(cxt, pc);
  }
  else if ( have(cxt, pc, "symbol") ) {
    set(cxt, expr, "type", "symbol_literal");
    set(cxt, expr, "value", sym(cxt, get(cxt, curr(cxt, pc), "value")));
    advance(cxt, pc);
  }
  return expr;
}

Object* parse_stmt(Object* cxt, Object *pc) {
  Object *stmt = 0;
  if ( have_set(cxt, pc, "begin_expr") ) {
    stmt = parse_expr_list(cxt, pc);
  }
  else if ( have(cxt, pc, "if") ) {
    mustbe(cxt, pc, "if");
    stmt = new_object(cxt, "Object");
    set(cxt, stmt, "type", "if_stmt");
    set(cxt, stmt, "cond", parse_group_expr(cxt, pc));
    set(cxt, stmt, "true_block", parse_block_expr(cxt, pc));
    if ( have(cxt, pc, "else") ) {
      mustbe(cxt, pc, "else");
      set(cxt, stmt, "false_block", parse_block_expr(cxt, pc));
    }
  }
  else if ( have(cxt, pc, "while") ) {
    mustbe(cxt, pc, "while");
    stmt = new_object(cxt, "Object");
    set(cxt, stmt, "type", "while_stmt");
    set(cxt, stmt, "cond", parse_group_expr(cxt, pc));
    set(cxt, stmt, "while_block", parse_block_expr(cxt, pc));
  }
  else if ( have(cxt, pc, "try") ) {
    mustbe(cxt, pc, "try");
    stmt = new_object(cxt, "Object");
    set(cxt, stmt, "type", "try_stmt");
    set(cxt, stmt, "try_block", parse_block_expr(cxt, pc));
    if ( have(cxt, pc, "catch") ) {
      mustbe(cxt, pc, "catch");
      set(cxt, stmt, "catch_block", parse_block_expr(cxt, pc));
    }
  }
  mustbe(cxt, pc, "semi");
  return stmt;
}

Object* parse_block_expr(Object* cxt, Object *pc) {
  Object *block = new_object(cxt, "Object");
  set(cxt, block, "type", "block_expr");

  if ( ! mustbe(cxt, pc, "block_open") ) return block;

  Object *begin_mark = mark(cxt, pc);

  while ( have(cxt, pc, "ident") || have(cxt, pc, "arg_ident") ) {
    push_slot(cxt, block, "args", curr(cxt, pc));
    advance(cxt, pc);
  }

  if ( ! have(cxt, pc, "pipe") ) { 
    set_undefined(cxt, block, "args");
    restore(cxt, pc, begin_mark);
  }
  else {
    advance(cxt, pc);
  }

  while ( ! have_set(cxt, pc, "end_block") ) {
    push_slot(cxt, block, "stmts", parse_stmt(cxt, pc));
  }
  mustbe(cxt, pc, "block_close");
  return block;
}

Object* parse_object_expr(Object* cxt, Object *pc) {
  Object *object = new_object(cxt, "Object");
  set(cxt, object, "type", "object_expr");

  if ( ! mustbe(cxt, pc, "object_open") ) return object;

  while ( ! have_set(cxt, pc, "end_object") ) {
    push_slot(cxt, object, "stmts", parse_stmt(cxt, pc));
  }
  mustbe(cxt, pc, "object_close");
  return object;
}

Object* parse_group_expr(Object* cxt, Object *pc) {
  Object *group = new_object(cxt, "Object");
  set(cxt, group, "type", "group");
  if ( ! mustbe(cxt, pc, "group_open") ) return group;
  set(cxt, group, "expr_list", parse_expr_list(cxt, pc));
  mustbe(cxt, pc, "group_close");
  return group;
}

void create_sets(Object* cxt, Object *pc) {
  Object *end_block = new_object(cxt, "Object");
  set(cxt, pc, "end_block", end_block);
  set(cxt, end_block, "block_close", get_true(cxt));
  set(cxt, end_block, "eof", get_true(cxt));

  Object *end_object = new_object(cxt, "Object");
  set(cxt, pc, "end_object", end_object);
  set(cxt, end_object, "object_close", get_true(cxt));
  set(cxt, end_object, "eof", get_true(cxt));

  Object *begin_expr = new_object(cxt, "Object");
  set(cxt, pc, "begin_expr", begin_expr);
  set(cxt, begin_expr, "ident", get_true(cxt));
  set(cxt, begin_expr, "arg_ident", get_true(cxt));
  set(cxt, begin_expr, "char_array", get_true(cxt));
  set(cxt, begin_expr, "operator", get_true(cxt));
  set(cxt, begin_expr, "number_literal", get_true(cxt));
  set(cxt, begin_expr, "symbol_literal", get_true(cxt));
  set(cxt, begin_expr, "group_open", get_true(cxt));
  set(cxt, begin_expr, "object_open", get_true(cxt));
};

Object *parse(Object* cxt, Object *sc) {
  Object *pc = new_object(cxt, "Object");
  set(cxt, pc, "errors", new_array(cxt));
  set(cxt, pc, "scan_context", sc);
  set(cxt, pc, "tokens", get(cxt, sc, "tokens"));
  set(cxt, pc, "index", new_boxed_int(cxt, 0));
  create_sets(cxt, pc);
  set(cxt, pc, "block", parse_block_expr(cxt, pc));
  return pc;
}

Object* native_parser_tokenize(Object* cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *scan_context = pop(cxt, stack);
  push(cxt, stack, parse(cxt, scan_context));
  return frame;
}

void init_parser_sys(Object *cxt) {
  Object *parser = context_get(cxt, "Parser");
  set(cxt, parser, "parse:", new_func(cxt, native_parser_tokenize));
}
