#include <stdio.h>
#include <ctype.h>

#include "parser.h"
#include "symbols.h"
#include "scanner.h"
#include "char_array.h"
#include "array.h"

fixnum new_parser(cxt_t *cxt, fixnum scanner) {
  fixnum parser = new_obj(cxt);
  set(cxt, parser, SYM_SCANNER, scanner);
  set(cxt, parser, SYM_CURR_TOKEN, scanner_next_token(cxt, scanner));
  set(cxt, parser, SYM_NEXT_TOKEN, scanner_next_token(cxt, scanner));
  set(cxt, parser, SYM_ERRORS, new_array(cxt));
  return parser;
}

fixnum parser_have(cxt_t *cxt, fixnum parser, fixnum type) {
  fixnum curr = get(cxt, parser, SYM_CURR_TOKEN);
  return get(cxt, curr, SYM_TYPE) == type;
}

fixnum parser_curr_token(cxt_t *cxt, fixnum parser) {
  return get(cxt, parser, SYM_CURR_TOKEN);
};

fixnum parser_curr_ident(cxt_t *cxt, fixnum parser) {
  return get(cxt, get(cxt, parser, SYM_CURR_TOKEN), SYM_IDENT);
};

void parser_advance(cxt_t *cxt, fixnum parser) {
  fixnum next_token = get(cxt, parser, SYM_NEXT_TOKEN);
  fixnum scanner = get(cxt, parser, SYM_SCANNER);
  set(cxt, parser, SYM_CURR_TOKEN, next_token);
  set(cxt, parser, SYM_NEXT_TOKEN, scanner_next_token(cxt, scanner));
}

void parser_add_error(cxt_t *cxt, fixnum parser, fixnum error_str) {
  fixnum errors = get(cxt, parser, SYM_ERRORS);
  fixnum error = new_obj(cxt);
  set(cxt, error, SYM_TEXT, error_str);
  set(cxt, error, SYM_TOKEN, get(cxt, parser, SYM_CURR_TOKEN));
  array_push(cxt, errors, error);
}

fixnum parser_mustbe(cxt_t *cxt, fixnum parser, fixnum type) {
  if ( parser_have(cxt, parser, type) ) {
    parser_advance(cxt, parser);
    return 1;
  }
  else {
    fixnum err_str = new_char_array(cxt, "Expected symbol of type ");
    fixnum err = char_array_append(cxt, err_str, new_char_array(cxt, get_sym_buf(type)));
    parser_add_error(cxt, parser, err);
    return 0;
  }
}

fixnum parser_parse_block(cxt_t *cxt, fixnum parser);
fixnum parser_parse_expr(cxt_t *cxt, fixnum parser, fixnum head);
fixnum parser_parse(cxt_t *cxt, fixnum parser);

fixnum parser_parse(cxt_t *cxt, fixnum parser) {
  return parser_parse_block(cxt, parser);
}

fixnum parser_parse_block(cxt_t *cxt, fixnum parser) {
  fixnum block = new_obj(cxt);
  fixnum args = new_array(cxt);
  fixnum stmts = new_array(cxt);
  fixnum stmt_num = 0;
  set(cxt, block, SYM_ARGS, args);
  set(cxt, block, SYM_STMTS, stmts);
  set(cxt, block, SYM_TYPE, SYM_BLOCK);
  if ( ! parser_mustbe(cxt, parser, SYM_BRACE_LEFT) ) {
    return block;
  }
  while( 
    ! parser_have(cxt, parser, SYM_EOF) && 
    ! parser_have(cxt, parser, SYM_BRACE_RIGHT) 
  ) {
    array_push(cxt, stmts, parser_parse_expr(cxt, parser, 0));
    if ( parser_have(cxt, parser, SYM_SEMI) ) {
      parser_advance(cxt, parser);
      stmt_num += 1;
    }
    else if (stmt_num == 0 && parser_have(cxt, parser, SYM_PIPE)) {
      parser_advance(cxt, parser);
      set(cxt, block, SYM_ARGS, array_pop(cxt, stmts));
    }
    else {
      break;
    }
  }
  parser_mustbe(cxt, parser, SYM_BRACE_RIGHT);
  return block;
}

fixnum ident_has_arg(cxt_t *cxt, fixnum ident) {
  fixnum len = char_array_len(cxt, ident);
  if ( len > 0 ) {
    char last_char = char_array_get_at(cxt, ident, len-1);
    if ( last_char == ':' ) {
      return 1;
    }
    char first_char = char_array_get_at(cxt, ident, 0);
    if ( ! isalpha(first_char) ) {
      return 1;
    }
  }
  return 0;
}

fixnum parser_parse_expr(cxt_t *cxt, fixnum parser, fixnum head) {
  if ( parser_have(cxt, parser, SYM_IDENT) ) {
    fixnum call = new_obj(cxt);
    fixnum ident = parser_curr_ident(cxt, parser);
    if ( head == 0 ) {
      head = call;
    }
    set(cxt, call, SYM_TYPE, SYM_CALL);
    set(cxt, call, SYM_IDENT, ident);
    parser_advance(cxt, parser);
    if ( ident_has_arg(cxt, ident) ) {
      fixnum arg = parser_parse_expr(cxt, parser, head);
      set(cxt, call, SYM_ARG, arg);
    }

    if ( parser_have(cxt, parser, SYM_EQUALS) ) {
      fixnum eq = new_obj(cxt);
      parser_advance(cxt, parser);
      set(cxt, eq, SYM_TYPE, SYM_EQUALS);
      set(cxt, eq, SYM_LHS, head);
      set(cxt, eq, SYM_RHS, parser_parse_expr(cxt, parser, eq));
    }
    else if ( parser_have(cxt, parser, SYM_IDENT) ) {
      set(cxt, call, SYM_NEXT, parser_parse_expr(cxt, parser, head));
    }
    return head;
  }
  else if (parser_have(cxt, parser, SYM_BRACE_LEFT)) {
    return parser_parse_block(cxt, parser);
  }
  else if (parser_have(cxt, parser, SYM_STRING)) {
    fixnum string = parser_curr_token(cxt, parser);
    parser_advance(cxt, parser);
    return string;
  }
  else if (parser_have(cxt, parser, SYM_NUMBER)) {
    fixnum number = parser_curr_token(cxt, parser);
    parser_advance(cxt, parser);
    return number;
  }
  else if (parser_have(cxt, parser, SYM_PAREN_LEFT)) {
    parser_advance(cxt, parser);
    fixnum expr = parser_parse_expr(cxt, parser, 0);
    parser_mustbe(cxt, parser, SYM_PAREN_RIGHT);
    return expr;
  }
  else {
    fixnum err_str = new_char_array(cxt, "Unexpected token");
    parser_add_error(cxt, parser, err_str);
    return 0;
  }
}

void print_indent(cxt_t *cxt, fixnum indent) {
  for(fixnum i = 0; i<indent; ++i) {
    fprintf(stdout, " ");
  }
}

void parser_print_ast(cxt_t *cxt, fixnum ast, fixnum indent) {
  if ( ast != 0 ) {
    fixnum type = get(cxt, ast, SYM_TYPE);
    if ( type == SYM_BLOCK ) {
      print_indent(cxt, indent);
      fprintf(stdout, "type: %s\n", get_sym_buf(type));

      print_indent(cxt, indent);
      fprintf(stdout, "args: \n");
      parser_print_ast(cxt, get(cxt, ast, SYM_ARGS), indent+1);

      print_indent(cxt, indent);
      fprintf(stdout, "stmts: \n");
      parser_print_ast(cxt, get(cxt, ast, SYM_STMTS), indent+1);
    }
    else {
      print_indent(cxt, indent);
      if ( type != 0 ) {
        fprintf(stdout, "type: %s\n", get_sym_buf(type));
        dump(cxt, ast);
      }
    }
  }
  else {
    fprintf(stdout, "null\n");
  };
};

void parser_print_errors(cxt_t *cxt, fixnum errors) {
  if ( array_len(cxt, errors) > 0 ) {
    fixnum err = array_pop(cxt, errors);
    fixnum err_string = get(cxt, err, SYM_TEXT);
    fixnum token = get(cxt, err, SYM_TOKEN);
    fixnum line_num = get(cxt, token, SYM_LINE_NUM);
    fixnum char_num = get(cxt, token, SYM_CHAR_NUM);
    fprintf(stderr, "%s:%d:%d: error %s\n", filename, line_num, char_num, get_buf(cxt, err_string));
    return 0;
  }
};

void parser_parse(cxt_t *cxt, char const* filename) {
  fixnum file    = new_file(cxt, new_char_array(cxt, filename));
  fixnum scanner = new_scanner(cxt, file);
  fixnum parser  = new_parser(cxt, scanner);
  fixnum result  = new_obj(cxt);
  set(cxt, result, SYM_AST, ast);
  fixnum ast     = parser_parse(cxt, parser);
  fixnum errors  = get(cxt, parser, SYM_ERRORS);
  if ( array_len(cxt, errors) > 0 ) {
    parser_print_errors(cxt, errors);
    return 0;
  }
  else {
    return ast;
  }
}
