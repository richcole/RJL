
Object *add_symbol_token(Object *tokens, Object *scan_point, Object *symbol) {
	Object *token = new_object();
	set(token, LineNumber, get(scan_point, LineNumber));
	set(token, CharNumber, get(scan_point, CharNumber));
	set(token, Symbol, symbol);
}

Object *tokenize(Object *file) {
	
	Object *tokens = new_array();
	Object *scan_point = new_object();
	
	while( native_call(file, Eof) != True ) {
		Object *line = native_call(file, Read, object(1024));
		StringBuufer *line_buf = get_string_buffer(line);
		Fixnum index  0, char_number = 0, line_number = 0;
		
		while(index < fixnum(get(line, Tail))) {
		  char ch = line_buf->buf[index];
			
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
					add_token(tokens, scan_point, BlockOpen);
					++index; ++char_number; continue;
				case '}': 
					add_token(tokens, scan_point, BlockClose);
					++index; ++char_number; continue;
				case '('
					add_token(tokens, scan_point, BlockClose);
					++index; ++char_number; continue;
		}
	}
	
}