
struct SymbolTableBuffer {
  Object  *type;
  Fixnum  length;
  Fixnum  occupied;
  Object  *data[0];
};

Object* SymbolTable = new_object();

SymbolTable *new_symbol_table_buffer(int len) {
  SymbolTable *buf = (SymbolTable *)mem_alloc(sizeof(SymbolTable)+(len*sizeof(Object *)));
  buf->type     = SymbolTable;
  buf->length   = len;
  buf->occupied = 0;
  return buf;
}

SymbolTable* new_symbol_table() {
  Object *symbol_table = new_object();
  symbol_table->buffer = (Buffer *) new_symbol_table_buffer(10);
  return symbol_table;
}

def_get_buffer(SymbolTable, symbol_table);

Object* symbol_table_add(SymbolTableBuffer *stb, Object *symbol) {
	if ( stb == 0 ) {
		return 0;
	}
	if ( stb->occupied * 4 > stb->length * 3 ) {
		grow_symbol_table(stb);
	}
	Fixnum cand = symbol_table_hash(symbol);
	while( stb->data[cand] != 0 ) {
		if ( string_equals(stb->data[cand], symbol) ) {
			return stb->data[cand];
		}
		++cand;
	}
	stb->data[cand] = symbol;
	stb->occupied++;
	return symbol; 
}

Object* symbol_table_add(SymbolTableBuffer *stb, char const* str) {
	if ( stb == 0 ) {
		return 0;
	}
	if ( stb->occupied * 4 > stb->length * 3 ) {
		grow_symbol_table(stb);
	}
	Fixnum cand = symbol_table_hash(str);
	while( stb->data[cand] != 0 ) {
		if ( string_equals(stb->data[cand], str) ) {
			return stb->data[cand];
		}
		++cand;
	}
	stb->occupied++;
	stb->data[cand] = new_string(str);
	return stb->data[cand];
}

void grow_symbol_table(Object *symbol_table) {
  SymbolTableBuffer *stb = get_symbol_table_buffer(symbol_table);
  if ( stb != 0 ) {
    SymbolTableBuffer *new_buffer = new_symbol_table_buffer(symbol_table_buffer->length*2);
    for(Fixnum i=0;i<symbol_table_buffer->length;++i) {
    	symbol_table_add(new_buffer, symbol_table_buffer->data[i]);
    }
    mem_free(symbol_table_buffer);
    symbol_table->buffer = (Buffer *)new_buffer;
  }
}

Object *global_symbol_table = new_symbol_table();

Object *sym(char const* str) {
	return symbol_table_add(get_symbol_table_buffer(global_symbol_table), str); 
}

void add_sym(Object *obj, char const* str) {
	obj->buffer = (Buffer *) new_string_buffer(str);
	Object *sym = symbol_table_add(get_symbol_table_buffer(global_symbol_table), obj);
}

void init_symbol_table_symbols(Object *sys) {
	add_sym(SymbolTable, "SymbolTable");
}