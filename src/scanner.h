
Object *LineNumber = new_object();
Object *CharNumber = new_object();
Object *Symbol     = new_object();
Object *Eof        = new_object();
Object *Read       = new_object();
Object *BlockOpen  = new_object();
Object *BlockClose = new_object();
Object *GroupOpen  = new_object();
Object *GroupClose = new_object();

void init_scanner_symbols() {
  add_sym(LineNumber,    "line_number");
  add_sym(CharNumber,    "char_number");
  add_sym(Symbol,        "symbol");
  add_sym(Eof,           "eof");
  add_sym(Read,          "read");
  add_sym(BlockOpen,     "block_open");
  add_sym(BlockClose,    "block_close");
  add_sym(GroupOpen,     "group_open");
  add_sym(GroupClose,    "group_close");
};

void push_token(Object *tokens, Object *scan_point, Object *symbol) {
	Object *token = new_object();
	set(token, LineNumber, get(scan_point, LineNumber));
	set(token, CharNumber, get(scan_point, CharNumber));
	set(token, Symbol, symbol);
  push(tokens, token);
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

Object *tokenize(Object *file) {
	
	Object *tokens = new_array();
	Object *scan_point = new_object();
	
	while( native_call(file, Eof) == False ) {
		Object *line = native_call(file, Read, object(1024));
		StringBuffer *line_buf = get_string_buffer(line);
		Fixnum index = 0, char_number = 0, line_number = 0;
		
		while(index < line_buf->length) {
		  char ch = line_buf->data[index];
			
			if ( is_white_space(ch) ) {
				++index; 
				++char_number;
				continue;
			}
			
			if ( is_line_ending(ch) ) {
				++line_number;
				char_number = 0;
				continue;
			}
			
			set(scan_point, LineNumber, object(line_number));
			set(scan_point, CharNumber, object(char_number));
			
			switch(ch) {
          case '{': 
            push_token(tokens, scan_point, BlockOpen);
            ++index; ++char_number; continue;
          case '}': 
            push_token(tokens, scan_point, BlockClose);
            ++index; ++char_number; continue;
          case '(':
            push_token(tokens, scan_point, GroupOpen);
            ++index; ++char_number; continue;
          case ')':
            push_token(tokens, scan_point, GroupClose);
            ++index; ++char_number; continue;
      }
    }
	}

  return tokens;
}
