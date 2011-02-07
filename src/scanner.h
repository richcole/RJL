
Object *LineNumber = new_object();
Object *CharNumber = new_object();
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
Object *Number     = new_object();
Object *Line       = new_object();
Object *Ident      = new_object();

void init_scanner_symbols() {
  add_sym(LineNumber,    "line_number");
  add_sym(CharNumber,    "char_number");
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
  add_sym(Number,        "number");
  add_sym(Line,          "line");
  add_sym(Ident,         "ident");
};

void scan_context_push_token(Object *sc, Object *type) {
	Object *token = new_object();
	set(token, sym("line_number"), get(sc, sym("line_number")));
  set(token, sym("char_number"), get(sc, sym("char_number")));
	set(token, sym("type"),        type);
  set(token, sym("value"),       
    string_substring(get(sc, sym("line")), 
      fixnum(get(sc, sym("token_start"))), 
      fixnum(get(sc, sym("token_end")))
    )
  );
  push(get(sc, sym("tokens")), token);
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

Object *new_scan_context(Object *file) {
  Object *sc = new_object();
  set(sc, sym("file"),        file);
  set(sc, sym("line"),        new_string(1024));
  set(sc, sym("line_number"), object(0));
  set(sc, sym("char_number"), object(0));
  set(sc, sym("index"),       object(0));
  set(sc, sym("token_start"),   object(0));
  set(sc, sym("token_end"),     object(0));
  set(sc, sym("tokens"),      new_array());
  return sc;
};

Object *scan_context_eof(Object *sc) {
  Object *file = get(sc, sym("file"));
  return native_call(file, sym("eof"));
};

void scan_context_read_line(Object *sc) {
  Object *file = get(sc, sym("file"));
  Object *line = get(sc, sym("line"));
  native_call(line, sym("shift:"), get(sc, sym("index")));
  set(sc, sym("index"), 0);
  native_call(file, sym("read:into:offset:length:"), 
    line, object(string_length(line)), 
    object(string_reserve(line) - string_length(line))
  );
  set(sc, sym("line_number"), object(fixnum(get(sc, sym("line_number")))+1));
  set(sc, sym("char_number"), object(0));
};

Fixnum scan_context_line_is_exhausted(Object *sc) {
  StringBuffer *buf = get_string_buffer(get(sc, sym("line")));
  if ( buf != 0 && buf->length > fixnum(get(sc, sym("index"))) ) {
    return 0;
  }
  else {
    return 1;
  }
}

char scan_context_curr(Object *sc) {
  Object *line  = get(sc, sym("line"));
  Fixnum  index = fixnum(get(sc, sym("index")));
  return string_get_at(line, index);
}

char scan_context_next(Object *sc) {
  Object *line  = get(sc, sym("line"));
  Fixnum  index = fixnum(get(sc, sym("index")))+1;
  return string_get_at(line, index);
}

Fixnum scan_context_has_next(Object *sc) {
  Object *line  = get(sc, sym("line"));
  Fixnum  index = fixnum(get(sc, sym("index")));
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
  set(sc, sym("char_number"), object(0));
}

char scan_context_advance(Object *sc) {
  scan_context_incr(sc, "char_number");
  scan_context_incr(sc, "token_end");
  scan_context_incr(sc, "index");
  return scan_context_curr(sc);
}

void scan_context_mark(Object *sc) {
  set(sc, sym("token_start"), get(sc, sym("index")));
  set(sc, sym("token_end"),   get(sc, sym("index")));
}

Fixnum is_digit(char c) {
  return c >= '0' && c <= '9';
};

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
        scan_context_push_token(sc, sym("string"));
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
        scan_context_push_token(sc, Number);
        continue;
      }
      
      if ( is_ident_start(ch) ) {
        if ( ! (ch = scan_context_advance(sc)) ) break;
        while( is_ident_continue(ch) ) {
          if ( ! (ch = scan_context_advance(sc)) ) break;
        }
        scan_context_push_token(sc, Ident);
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

void init_scanner_sys(Object *sys) {
  Object *scanner_object = new_object();
  set(sys, sym("Scanner"), scanner_object);
  set(scanner_object, sym("tokenize:"), new_func(native_scanner_tokenize));
}
