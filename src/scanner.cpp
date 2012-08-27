#include "scanner.h"
#include "symbols.h"
#include "char_array.h"

#include <stdio.h>
#include <ctype.h>

fixnum new_file(cxt_t *cxt, fixnum filename) {
  fixnum file = new_obj(cxt);
  set(cxt, file, SYM_FILENAME, filename);
  FILE *fp = fopen(char_array_get_buf(cxt, filename), "r");
  set_buf(cxt, file, (char *)&fp, sizeof(fp));
  return file;
}

FILE* get_file_ptr(cxt_t *cxt, fixnum file) {
  return *(FILE **) get_buf(cxt, file);
};

fixnum file_next_char(cxt_t *cxt, fixnum file) {
  return fgetc(get_file_ptr(cxt, file));
};

void file_close(cxt_t *cxt, fixnum file) {
  fclose(get_file_ptr(cxt, file));
};

fixnum new_scanner(cxt_t *cxt, fixnum file) {
  fixnum scanner = new_obj(cxt);
  set(cxt, scanner, SYM_FILE, file);
  set(cxt, scanner, SYM_LINE_NUM, 1);
  set(cxt, scanner, SYM_CHAR_NUM, 0);
  return scanner;
}

fixnum char_is_eol(cxt_t *cxt, fixnum c) {
  if ( c == '\n' ) {
    return 1;
  }
  else {
    return 0;
  }
}

fixnum char_is_digit(cxt_t *cxt, fixnum c) {
  return c >= '0' && c <= '9';
}

fixnum char_is_space(cxt_t *cxt, fixnum c) {
  if ( c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\n' ) {
    return true;
  }
  return false;
}

fixnum char_is_special_punct(cxt_t *cxt, fixnum c) {
  return c == '{' || c == '}' || c == ',' || c == ';' || c == '(' || c == ')';
}

fixnum char_is_ident(cxt_t *cxt, fixnum c) {
  return ! char_is_space(cxt, c) && ! char_is_special_punct(cxt, c);
}

fixnum scanner_advance(cxt_t *cxt, fixnum scanner) {
  fixnum next_char = get(cxt, scanner, SYM_NEXT_CHAR);
  fixnum file = get(cxt, scanner, SYM_FILE);
  fixnum curr_char = next_char;

  if ( curr_char == 0 ) {
    curr_char = file_next_char(cxt, file);
  }
  next_char = file_next_char(cxt, file);
  set(cxt, scanner, SYM_CURR_CHAR, curr_char);
  set(cxt, scanner, SYM_NEXT_CHAR, next_char);
  if ( char_is_eol(cxt, curr_char) ) {
    incr(cxt, scanner, SYM_LINE_NUM);
    set(cxt, scanner, SYM_CHAR_NUM, 0);
  }
  else {
    incr(cxt, scanner, SYM_CHAR_NUM);
  }
  return curr_char;
};

fixnum scanner_next_char(cxt_t *cxt, fixnum scanner) {
  return get(cxt, scanner, SYM_NEXT_CHAR);
}

fixnum scanner_curr_char(cxt_t *cxt, fixnum scanner) {
  return get(cxt, scanner, SYM_CURR_CHAR);
}

fixnum new_token(cxt_t *cxt) {
  return new_obj(cxt);
}

fixnum scanner_next_token(cxt_t *cxt, fixnum scanner) {
  fixnum curr_char = scanner_advance(cxt, scanner);
  fixnum token     = new_token(cxt);
  fixnum ident     = new_char_array(cxt);

  while ( char_is_space(cxt, curr_char) ) {
    curr_char = scanner_advance(cxt, scanner);
  }

  set(cxt, token, SYM_LINE_NUM, get(cxt, scanner, SYM_LINE_NUM));
  set(cxt, token, SYM_CHAR_NUM, get(cxt, scanner, SYM_CHAR_NUM));
  set(cxt, token, SYM_IDENT, ident);

  char_array_put_char(cxt, ident, curr_char);

  if ( curr_char == ',' ) {
    return set(cxt, token, SYM_TYPE, SYM_COMMA);
  }

  if ( curr_char == '.' ) {
    return set(cxt, token, SYM_TYPE, SYM_PERIOD);
  }

  if ( curr_char == ';' ) {
    return set(cxt, token, SYM_TYPE, SYM_SEMI);
  }

  if ( curr_char == '{' ) {
    return set(cxt, token, SYM_TYPE, SYM_BRACE_LEFT);
  }

  if ( curr_char == '}' ) {
    return set(cxt, token, SYM_TYPE, SYM_BRACE_RIGHT);
  }

  if ( curr_char == '(' ) {
    return set(cxt, token, SYM_TYPE, SYM_PAREN_LEFT);
  }

  if ( curr_char == ')' ) {
    return set(cxt, token, SYM_TYPE, SYM_PAREN_RIGHT);
  }

  if ( curr_char == '|' ) {
    return set(cxt, token, SYM_TYPE, SYM_PIPE);
  }

  if ( curr_char == '=' ) {
    return set(cxt, token, SYM_TYPE, SYM_EQUALS);
  }

  if ( curr_char == -1 ) {
    return set(cxt, token, SYM_TYPE, SYM_EOF);
  }

  if ( curr_char == '"' ) {
    set_buf_tail(cxt, ident, 0);
    set(cxt, token, SYM_TYPE,  SYM_STRING);
    while( 
      scanner_next_char(cxt, scanner) != '"' && 
      scanner_next_char(cxt, scanner) != -1 
    ) {
      scanner_advance(cxt, scanner);
      char_array_put_char(cxt, ident, scanner_curr_char(cxt, scanner));
    }
    scanner_advance(cxt, scanner); // skip the quote
    return token;
  }

  set(cxt, token, SYM_TYPE,  SYM_IDENT);
  while ( char_is_ident(cxt, scanner_next_char(cxt, scanner)) ) {
    scanner_advance(cxt, scanner);
    char_array_put_char(cxt, ident, scanner_curr_char(cxt, scanner));
  }

  if ( char_is_digit(cxt, char_array_get_at(cxt, ident, 0)) ) {
    set(cxt, token, SYM_TYPE, SYM_NUMBER);
  }

  return token;
}
  
void scanner_scan(cxt_t *cxt, char const* filename) {
  fixnum file    = new_file(cxt, new_char_array(cxt, filename));
  fixnum scanner = new_scanner(cxt, file);
  fixnum token   = 0;
  fixnum ident   = 0;
  char const* type_buf;
  char const* ident_buf;
  fixnum line_num, char_num;

  while(1) {
    token = scanner_next_token(cxt, scanner);
    ident = get(cxt, token, SYM_IDENT);
    type_buf = get_sym_buf(get(cxt, token, SYM_TYPE));
    ident_buf = ident ? char_array_get_buf(cxt, ident) : "(null)";
    line_num = get(cxt, token, SYM_LINE_NUM);
    char_num = get(cxt, token, SYM_CHAR_NUM);
    fprintf(stdout, "(%d,%d) type=%s ident=%s \n", line_num, char_num, type_buf, ident_buf);
    if ( get(cxt, token, SYM_TYPE) == SYM_EOF ) {
      break;
    }
  }
}
