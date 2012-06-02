#include "scanner.h"
#include "std.h"
#include "setter.h"
#include "native.h"
#include "file.h"

void scan_context_push_token(Object *cxt, Object *sc, char const* type, Fixnum offset) {
  Object *token = new_object(cxt, "Object");
  Fixnum token_start = boxed_int_to_fixnum(cxt, get(cxt, sc, "token_start"));
  Fixnum line_start  = boxed_int_to_fixnum(cxt, get(cxt, sc, "line_start"));
  set(cxt, token, "line_number", get(cxt, sc, "line_number"));
  set(cxt, token, "char_number", new_boxed_int(cxt, token_start - line_start));
  set(cxt, token, "type",        type);
  set(cxt, token, "value",       
      char_array_subchar_array(cxt, get(cxt, sc, "line"), 
		       boxed_int_to_fixnum(cxt, get(cxt, sc, "token_start")) + offset, 
		       boxed_int_to_fixnum(cxt, get(cxt, sc, "token_end"))
    )
  );
  push(cxt, get(cxt, sc, "tokens"), token);
};

void scan_context_push_token(Object *cxt, Object *sc, char const* type) {
  scan_context_push_token(cxt, sc, type, 0);
};

void scan_context_push_char_array_token(Object *cxt, Object *sc, Object *type) {
  Object *token = new_object(cxt, "Object");
  Fixnum token_start = boxed_int_to_fixnum(cxt, get(cxt, sc, "token_start"));
  Fixnum line_start  = boxed_int_to_fixnum(cxt, get(cxt, sc, "line_start"));
  set(cxt, token, "line_number", get(cxt, sc, "line_number"));
  set(cxt, token, "char_number", new_boxed_int(cxt, token_start - line_start));
  set(cxt, token, "type",        type);
  set(cxt, token, "value",       
      char_array_unescape(cxt, get(cxt, sc, "line"), 
		       boxed_int_to_fixnum(cxt, get(cxt, sc, "token_start"))+1, 
		       boxed_int_to_fixnum(cxt, get(cxt, sc, "token_end"))-1
    )
  );
  push(cxt, get(cxt, sc, "tokens"), token);
};

Fixnum is_white_space(char c) {
  if ( c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' ) {
    return 1;
  }
  return 0;
}

Fixnum is_line_ending(char c) {
  if ( c == '\n' ) {
    return 1;
  }
  return 0;
}

Fixnum is_arg_ident(Object *cxt, Object *tok) {
  Object *value = get(cxt, tok, "value");
  return is_setter_slot(cxt, value) && 
    ! is_nocall_slot(cxt, value);
}

void detect_arg_ident(Object *cxt, Object *sc) {
  Object *tok = peek(cxt, get(cxt, sc, "tokens"));
  if (is_arg_ident(cxt, tok)) {
    set(cxt, tok, "type", "arg_ident");
  }
}

Object* get_reserved_word(Object *cxt, Object *sc, Object *word) {
  Object *result = get(cxt, get(cxt, sc, "reserved_words"), sym(cxt, word));
  if ( is_char_array(cxt, result) ) {
    return result;
  }
  else {
    return get_undefined(cxt);
  }
}

void detect_reserved_word(Object *cxt, Object *sc) {
  Object *tok = peek(cxt, get(cxt, sc, "tokens"));
  Object *reserved_word = get_reserved_word(cxt, sc, get(cxt, tok, "value"));
  if ( exists(cxt, reserved_word) ) {
    set(cxt, tok, "type", reserved_word);
  }
}

Object *new_scan_context(Object *cxt, Object *file) {
  Object *sc = new_object(cxt, "Object");
  set(cxt, sc, "file",        file);
  set(cxt, sc, "line",        new_char_array(cxt, 1024));
  set(cxt, sc, "line_number", new_boxed_int(cxt, 0));
  set(cxt, sc, "line_start", new_boxed_int(cxt, 0));
  set(cxt, sc, "char_number", new_boxed_int(cxt, 0));
  set(cxt, sc, "index",       new_boxed_int(cxt, 0));
  set(cxt, sc, "token_start",   new_boxed_int(cxt, 0));
  set(cxt, sc, "token_end",     new_boxed_int(cxt, 0));
  set(cxt, sc, "tokens",      new_array(cxt));

  Object *rw = new_object(cxt, "Object");
  set(cxt, sc, "reserved_words", rw);
  set(cxt, rw, "parent", get_undefined(cxt));
  set(cxt, rw, "if",     "if");
  set(cxt, rw, "else",   "else");
  set(cxt, rw, "while",  "while");
  set(cxt, rw, "try",    "try");
  set(cxt, rw, "finally","finally");
  set(cxt, rw, "catch",  "catch");
  
  return sc;
};

Object *scan_context_eof(Object *cxt, Object *sc) {
  Object *file = get(cxt, sc, "file");
  return native_call(cxt, file, "eof");
};

void scan_context_read_line(Object *cxt, Object *sc) {
  Object *file = get(cxt, sc, "file");
  Object *line = get(cxt, sc, "line");
  Fixnum token_start = boxed_int_to_fixnum(cxt, get(cxt, sc, "token_start"));
  Fixnum token_end = boxed_int_to_fixnum(cxt, get(cxt, sc, "token_end"));
  Fixnum index = boxed_int_to_fixnum(cxt, get(cxt, sc, "index"));
  native_call(cxt, line, "shift:", new_boxed_int(cxt, token_start));
  set(cxt, sc, "index", new_boxed_int(cxt, index - token_start));
  set(cxt, sc, "token_start", new_boxed_int(cxt, 0));
  set(cxt, sc, "token_end", new_boxed_int(cxt, token_end - token_start));
  native_call(cxt, file, "read:into:offset:length:", 
              line, new_boxed_int(cxt, char_array_length(cxt, line)), 
              new_boxed_int(cxt, char_array_reserve(cxt, line) - char_array_length(cxt, line))
  );
};

Fixnum scan_context_line_is_exhausted(Object *cxt, Object *sc) {
  CharArrayBuffer *buf = get_char_array_buffer(get(cxt, sc, "line"));
  if ( buf != 0 ) {
    if ( buf->length > boxed_int_to_fixnum(cxt, get(cxt, sc, "index")) ) {
      return 0;
    }
    else {
      return 1;
    }
  }
  else {
    return 1;
  }
}

char scan_context_curr(Object *cxt, Object *sc) {
  Object *line  = get(cxt, sc, "line");
  Fixnum  index = boxed_int_to_fixnum(cxt, get(cxt, sc, "index"));
  return char_array_get_at(cxt, line, index);
}

char scan_context_next(Object *cxt, Object *sc) {
  Object *line  = get(cxt, sc, "line");
  Fixnum  index = boxed_int_to_fixnum(cxt, get(cxt, sc, "index"))+1;
  return char_array_get_at(cxt, line, index);
}

Fixnum scan_context_has_next(Object *cxt, Object *sc) {
  Object *line  = get(cxt, sc, "line");
  Fixnum  index = boxed_int_to_fixnum(cxt, get(cxt, sc, "index"));
  if (  index+1 < char_array_length(cxt, line) ) {
    return 1;
  }
  else {
    return 0;
  }
}

void scan_context_incr(Object *cxt, Object *sc, char const* slot) {
  set(cxt, sc, slot, 
      new_boxed_int(cxt, boxed_int_to_fixnum(cxt, get(cxt, sc, slot))+1));
}

void scan_context_newline(Object *cxt, Object *sc) {
  scan_context_incr(cxt, sc, "line_number");
  set(cxt, sc, "char_number", new_boxed_int(cxt, 0));
  set(cxt, sc, "line_start", get(cxt, sc, "index"));
}

char scan_context_advance(Object *cxt, Object *sc) {
  scan_context_incr(cxt, sc, "char_number");
  scan_context_incr(cxt, sc, "token_end");
  scan_context_incr(cxt, sc, "index");

  if ( scan_context_line_is_exhausted(cxt, sc) ) {
    if ( is_false(cxt, scan_context_eof(cxt, sc)) ) {
      scan_context_read_line(cxt, sc);
    }
  }

  char c = scan_context_curr(cxt, sc);
  if ( c == '\n' ) {
    scan_context_newline(cxt, sc);
  }
  return c;
}

void scan_context_mark(Object *cxt, Object *sc) {
  set(cxt, sc, "token_start", get(cxt, sc, "index"));
  set(cxt, sc, "token_end",   get(cxt, sc, "index"));
}

Fixnum is_digit(char c) {
  return c >= '0' && c <= '9';
};

Fixnum is_punct(char c) {
  return  
    (c == '-') || (c == '<') || (c == '>') ||
    (c == '+') || (c == '&') || (c == '=') || 
    (c == '.') || (c == '*') || (c == '/') || 
    (c == '!')
   ;
}

Fixnum is_operator_start(char c) {
  return is_punct(c);
}

Fixnum is_operator_continue(char c) {
  return is_punct(c) || c == ':';
}

Fixnum is_ident_start(char c) {
  return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
    c == '_' || ( c == '^' ) ;
};

Fixnum is_ident_continue(char c) {
  return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
    c == '_' || c == ':';
};

Object *tokenize(Object *cxt, Object *frame, Object *file) {

  Object *sc = new_scan_context(cxt, file);
  Object *parent = new_object(cxt);

  set(cxt, parent, "frame", frame);
  set(cxt, parent, "sc", sc);

  Fixnum loop_count = 0;
  
  scan_context_read_line(cxt, sc);
  while(! (scan_context_line_is_exhausted(cxt, sc) && is_true(cxt, scan_context_eof(cxt, sc))) ) {
    char ch = scan_context_curr(cxt, sc);
    
    if ( is_white_space(ch) ) {
      scan_context_advance(cxt, sc);
      continue;
    }
    
    if ( is_line_ending(ch) ) {
      scan_context_advance(cxt, sc);
      continue;
    }
    
    scan_context_mark(cxt, sc);
    
    if ( (++loop_count) % 100 == 0 ) {
      context_mark_and_sweep(cxt, parent);
    }
    
    switch(ch) {
    case '{': 
      scan_context_push_token(cxt, sc, "block_open");
      scan_context_advance(cxt, sc);
      continue;
    case '}': 
      scan_context_push_token(cxt, sc, "block_close");
      scan_context_advance(cxt, sc);
      continue;
    case '(':
      if ( scan_context_next(cxt, sc) == '|' ) {
        scan_context_advance(cxt, sc);
        scan_context_push_token(cxt, sc, "object_open");
        scan_context_advance(cxt, sc);
      }
      else {
        scan_context_push_token(cxt, sc, "group_open");
        scan_context_advance(cxt, sc);
      }
      continue;
    case ')':
      scan_context_push_token(cxt, sc, "group_close");
      scan_context_advance(cxt, sc);
      continue;
    case '[':
      scan_context_push_token(cxt, sc, "array_open");
      scan_context_advance(cxt, sc);
      continue;
    case ']':
      scan_context_push_token(cxt, sc, "array_close");
      scan_context_advance(cxt, sc);
      continue;
    case '|':
      if ( scan_context_next(cxt, sc) == ')' ) {
        scan_context_advance(cxt, sc);
        scan_context_advance(cxt, sc);
        scan_context_push_token(cxt, sc, "object_close");
      }
      else if ( scan_context_next(cxt, sc) == '|' ) {
        scan_context_advance(cxt, sc);
        if ( scan_context_next(cxt, sc) == ':' ) {
          while( scan_context_next(cxt, sc) == ':' ) {
            scan_context_advance(cxt, sc);
          }
          scan_context_advance(cxt, sc);
          scan_context_push_token(cxt, sc, "ident");
          detect_arg_ident(cxt, sc);
          detect_reserved_word(cxt, sc);
        }
        else {
          scan_context_advance(cxt, sc);
          scan_context_push_token(cxt, sc, "operator");
        }
      }
      else {
        scan_context_advance(cxt, sc);
        scan_context_push_token(cxt, sc, "pipe");
      }
      continue;
    case ';':
      scan_context_push_token(cxt, sc, "semi");
      scan_context_advance(cxt, sc);
      continue;
    }
    
    if ( ch == '"' ) {
      ch = scan_context_advance(cxt, sc);
      while ( ch != '"' && ch != 0 ) {
        if ( ch == '\\' ) {
          scan_context_advance(cxt, sc);
        }
        ch = scan_context_advance(cxt, sc);
      }            
      scan_context_advance(cxt, sc);
      scan_context_push_char_array_token(cxt, sc, sym(cxt, "char_array"));
      continue;
    }
    
    if  (ch == '-' && is_digit(scan_context_next(cxt, sc))) {
      if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
    }        
    
    if ( is_digit(ch) ) {
      while(is_digit(ch)) {
        if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
      }
      if ( ch == '.' ) {
        if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
      }
      while(is_digit(ch)) {
        if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
      }
      if ( ch == 'e' || ch == 'E' ) {
        if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
        if ( ch == '-' && is_digit(scan_context_next(cxt, sc)) ) {
          if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
        }
        while(is_digit(ch)) {
          if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
        }
      }
      scan_context_push_token(cxt, sc, "number_literal");
      continue;
    }
    
    if ( ch == '#' ) {
      if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
      if ( is_ident_continue(ch) ) {
        while ( is_ident_continue(ch) ) {
          if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
        }
        scan_context_push_token(cxt, sc, "symbol", 1);
      }
      else {
        while( is_operator_continue(ch) ) {
          if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
        }
        scan_context_push_token(cxt, sc, "operator");
      }
    }
    
    if ( is_ident_start(ch) ) {
      if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
      while( is_ident_continue(ch) ) {
        if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
      }
      scan_context_push_token(cxt, sc, "ident");
      detect_arg_ident(cxt, sc);
      detect_reserved_word(cxt, sc);
    }
    
    if ( is_operator_start(ch) ) {
      if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
      while( is_operator_continue(ch) ) {
        if ( ! (ch = scan_context_advance(cxt, sc)) ) break;
      }
      scan_context_push_token(cxt, sc, "operator");
      detect_arg_ident(cxt, sc);
    }
  }

  scan_context_push_token(cxt, sc, "eof");

  return sc;
}

Object* native_scanner_tokenize(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *file = pop(cxt, stack);
  if ( ! is_file(cxt, file) ) {
    return new_exception(cxt, frame, "Expected a file argument");
  }
  push(cxt, stack, tokenize(cxt, frame, file));
  return frame;
}

void init_scanner_sys(Object *cxt) {
  Object *scanner_object = context_get(cxt, "Scanner");
  set(cxt, scanner_object, "tokenize:", new_func(cxt, native_scanner_tokenize));
}
