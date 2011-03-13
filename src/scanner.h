
Object *Eof        = new_object();
Object *Read       = new_object();
Object *BlockOpen  = new_object();
Object *BlockClose = new_object();
Object *GroupOpen  = new_object();
Object *GroupClose = new_object();
Object *ArrayOpen  = new_object();
Object *ArrayClose = new_object();
Object *Pipe       = new_object();
Object *Semi       = new_object();
Object *Line       = new_object();
Object *Ident      = new_object();
Object *Operator   = new_object();

void scan_context_push_token(Object *sc, Object *type) {
  Object *token = new_object();
  Fixnum token_start = fixnum(get(sc, "token_start"));
  Fixnum line_start  = fixnum(get(sc, "line_start"));
  set(token, "line_number", get(sc, "line_number"));
  set(token, "char_number", object(token_start - line_start));
  set(token, "type",        type);
  set(token, "value",       
      string_substring(get(sc, "line"), 
		       fixnum(get(sc, "token_start")), 
		       fixnum(get(sc, "token_end"))
    )
  );
  push(get(sc, "tokens"), token);
};

void scan_context_push_token(Object *sc, char const* type) {
  scan_context_push_token(sc, sym(type));
}

void scan_context_push_string_token(Object *sc, Object *type) {
  Object *token = new_object();
  Fixnum token_start = fixnum(get(sc, "token_start"));
  Fixnum line_start  = fixnum(get(sc, "line_start"));
  set(token, "line_number", get(sc, "line_number"));
  set(token, "char_number", object(token_start - line_start));
  set(token, "type",        type);
  set(token, "value",       
      string_substring(get(sc, "line"), 
		       fixnum(get(sc, "token_start"))+1, 
		       fixnum(get(sc, "token_end"))-1
    )
  );
  push(get(sc, "tokens"), token);
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

Fixnum is_arg_ident(Object *tok) {
  return is_setter_slot(get(tok, "value"));
}

void detect_arg_ident(Object *sc) {
  Object *tok = array_last(get(sc, "tokens"));
  if (is_arg_ident(tok)) {
    set(tok, "type", "arg_ident");
  }
}

Object* get_reserved_word(Object *sc, Object *word) {
  return get(get(sc, "reserved_words"), sym(word));
}

void detect_reserved_word(Object *sc) {
  Object *tok = array_last(get(sc, "tokens"));
  Object *reserved_word = get_reserved_word(sc, get(tok, "value"));
  if ( reserved_word != Undefined ) {
    set(tok, "type", reserved_word);
  }
}

Object *new_scan_context(Object *file) {
  Object *sc = new_object();
  set(sc, "file",        file);
  set(sc, "line",        new_string(1024));
  set(sc, "line_number", object(0));
  set(sc, "line_start", object(0));
  set(sc, "char_number", object(0));
  set(sc, "index",       object(0));
  set(sc, "token_start",   object(0));
  set(sc, "token_end",     object(0));
  set(sc, "tokens",      new_array());

  Object *rw = new_object();
  set(sc, "reserved_words", rw);
  set(rw, "if", "if");
  set(rw, "else", "else");
  set(rw, "while", "while");
  set(rw, "return", "return");
  
  return sc;
};

Object *scan_context_eof(Object *sc) {
  Object *file = get(sc, "file");
  return native_call(file, "eof");
};

void scan_context_read_line(Object *sc) {
  Object *file = get(sc, "file");
  Object *line = get(sc, "line");
  native_call(line, "shift:", get(sc, "index"));
  set(sc, "index", object(0));
  native_call(file, "read:into:offset:length:", 
    line, object(string_length(line)), 
    object(string_reserve(line) - string_length(line))
  );
  set(sc, "line_number", object(fixnum(get(sc, "line_number"))+1));
  set(sc, "char_number", object(0));
};

Fixnum scan_context_line_is_exhausted(Object *sc) {
  StringBuffer *buf = get_string_buffer(get(sc, "line"));
  if ( buf != 0 && buf->length > fixnum(get(sc, "index")) ) {
    return 0;
  }
  else {
    return 1;
  }
}

char scan_context_curr(Object *sc) {
  Object *line  = get(sc, "line");
  Fixnum  index = fixnum(get(sc, "index"));
  return string_get_at(line, index);
}

char scan_context_next(Object *sc) {
  Object *line  = get(sc, "line");
  Fixnum  index = fixnum(get(sc, "index"))+1;
  return string_get_at(line, index);
}

Fixnum scan_context_has_next(Object *sc) {
  Object *line  = get(sc, "line");
  Fixnum  index = fixnum(get(sc, "index"));
  if (  index+1 < string_length(line) ) {
    return 1;
  }
  else {
    return 0;
  }
}

void scan_context_incr(Object *sc, char const* sym_str) {
  set(sc, sym(sym_str), object(fixnum(get(sc, sym(sym_str)))+1));
}

void scan_context_newline(Object *sc) {
  scan_context_incr(sc, "line_number");
  set(sc, "char_number", object(0));
  set(sc, "line_start", get(sc, "index"));
}

char scan_context_advance(Object *sc) {
  scan_context_incr(sc, "char_number");
  scan_context_incr(sc, "token_end");
  scan_context_incr(sc, "index");
  char c = scan_context_curr(sc);
  if ( c == '\n' ) {
    scan_context_newline(sc);
  }
  return c;
}

void scan_context_mark(Object *sc) {
  set(sc, "token_start", get(sc, "index"));
  set(sc, "token_end",   get(sc, "index"));
}

Fixnum is_digit(char c) {
  return c >= '0' && c <= '9';
};

Fixnum is_punct(char c) {
  return  (c == '-') || (c == '<')  || (c == '>') ||
    (c == '+') || (c == '&')  || (c == '=');
}

Fixnum is_operator_start(char c) {
  return is_punct(c);
}

Fixnum is_operator_continue(char c) {
  return is_punct(c);
}

Fixnum is_ident_start(char c) {
  return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
    c == '_';
};

Fixnum is_ident_continue(char c) {
  return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
    c == '_' || c == ':';
};

Object *tokenize(Object *file) {
  
  Object *sc = new_scan_context(file);
  
  while( scan_context_eof(sc) == False ) {
    scan_context_read_line(sc);
    
    while(! scan_context_line_is_exhausted(sc)) {
      char ch = scan_context_curr(sc);

      if ( is_white_space(ch) ) {
        scan_context_advance(sc);
        continue;
      }
      
      if ( is_line_ending(ch) ) {
        scan_context_advance(sc);
        continue;
      }

      scan_context_mark(sc);

      switch(ch) {
          case '{': 
            scan_context_push_token(sc, BlockOpen);
            scan_context_advance(sc);
            continue;
          case '}': 
            scan_context_push_token(sc, BlockClose);
            scan_context_advance(sc);
            continue;
          case '(':
            scan_context_push_token(sc, GroupOpen);
            scan_context_advance(sc);
            continue;
          case ')':
            scan_context_push_token(sc, GroupClose);
            scan_context_advance(sc);
            continue;
          case '[':
            scan_context_push_token(sc, ArrayOpen);
            scan_context_advance(sc);
            continue;
          case ']':
            scan_context_push_token(sc, ArrayClose);
            scan_context_advance(sc);
            continue;
          case '|':
            scan_context_push_token(sc, Pipe);
            scan_context_advance(sc);
            continue;
          case ';':
            scan_context_push_token(sc, Semi);
            scan_context_advance(sc);
            continue;
      }

      if ( ch == '"' ) {
        ch = scan_context_advance(sc);
        while ( ch != '"' && ch != 0 ) {
          if ( ch == '\\' ) {
            scan_context_advance(sc);
          }
          ch = scan_context_advance(sc);
        }            
        scan_context_advance(sc);
        scan_context_push_string_token(sc, sym("string"));
        continue;
      }

      if  (ch == '-' && is_digit(scan_context_next(sc))) {
        if ( ! (ch = scan_context_advance(sc)) ) break;
      }        

      if ( is_digit(ch) ) {
        while(is_digit(ch)) {
          if ( ! (ch = scan_context_advance(sc)) ) break;
        }
        if ( ch == '.' ) {
          if ( ! (ch = scan_context_advance(sc)) ) break;
        }
        while(is_digit(ch)) {
          if ( ! (ch = scan_context_advance(sc)) ) break;
        }
        if ( ch == 'e' || ch == 'E' ) {
          if ( ! (ch = scan_context_advance(sc)) ) break;
          if ( ch == '-' && is_digit(scan_context_next(sc)) ) {
            if ( ! (ch = scan_context_advance(sc)) ) break;
          }
          while(is_digit(ch)) {
            if ( ! (ch = scan_context_advance(sc)) ) break;
          }
        }
        scan_context_push_token(sc, "number");
        continue;
      }
      
      if ( is_ident_start(ch) ) {
        if ( ! (ch = scan_context_advance(sc)) ) break;
        while( is_ident_continue(ch) ) {
          if ( ! (ch = scan_context_advance(sc)) ) break;
        }
        scan_context_push_token(sc, Ident);
        detect_arg_ident(sc);
        detect_reserved_word(sc);
      }

      if ( is_operator_start(ch) ) {
        if ( ! (ch = scan_context_advance(sc)) ) break;
        while( is_operator_continue(ch) ) {
          if ( ! (ch = scan_context_advance(sc)) ) break;
        }
        scan_context_push_token(sc, Operator);
      }
    }
  }

  return sc;
}

Object* native_scanner_tokenize(Object *frame, Object *self) {
  Object *file = pop(get(frame, Stack));
  if ( ! is_file(file) ) {
    return new_exception(frame, "Expected a file argument");
  }
  push(get(frame, Stack), tokenize(file));
  return frame;
}

void init_scanner_symbols() {
  add_sym(Eof,           "eof");
  add_sym(Read,          "read");
  add_sym(BlockOpen,     "block_open");
  add_sym(BlockClose,    "block_close");
  add_sym(GroupOpen,     "group_open");
  add_sym(GroupClose,    "group_close");
  add_sym(ArrayOpen,     "array_open");
  add_sym(ArrayClose,    "array_close");
  add_sym(Pipe,          "pipe");
  add_sym(Semi,          "semi");
  add_sym(Line,          "line");
  add_sym(Ident,         "ident");
  add_sym(Operator,      "operator");
};

void init_scanner_sys(Object *sys) {
  Object *scanner_object = new_object();
  set(sys, "Scanner", scanner_object);
  set(scanner_object, "tokenize:", new_func(native_scanner_tokenize));
}
