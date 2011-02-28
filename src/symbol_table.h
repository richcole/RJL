
struct SymbolTableBuffer {
  Object  *type;
  Fixnum  length;
  Fixnum  occupied;
  Object  *data[0];
};

Object* SymbolTable = new_object();

SymbolTableBuffer *new_symbol_table_buffer(int len) {
  SymbolTableBuffer *buf = (SymbolTableBuffer *) 
    mem_alloc(sizeof(SymbolTableBuffer)+(len*sizeof(Object *)));
  buf->type     = SymbolTable;
  buf->length   = len;
  buf->occupied = 0;
  return buf;
}

Object* new_symbol_table() {
  Object *symbol_table = new_object();
  symbol_table->buffer = (Buffer *) new_symbol_table_buffer(10);
  return symbol_table;
}

def_get_buffer(SymbolTable, symbol_table);

Object* symbol_table_add(SymbolTableBuffer *stb, Object *symbol);

SymbolTableBuffer *grow_symbol_table(Object *symbol_table) {
  SymbolTableBuffer *stb = get_symbol_table_buffer(symbol_table);
  if ( stb != 0 && stb->occupied * 4 > stb->length * 3 ) {
    SymbolTableBuffer *new_buffer = new_symbol_table_buffer(stb->length*2);
    for(Fixnum i=0;i<stb->length;++i) {
    	symbol_table_add(new_buffer, stb->data[i]);
    }
    mem_free(stb);
    stb = new_buffer;
    symbol_table->buffer = (Buffer *) stb;
  }
  return stb;
}

Fixnum symbol_table_hash(Object *symbol) {
  StringBuffer *sb = get_string_buffer(symbol);
  Fixnum hash = 0;
  for(Fixnum i=0;i<sb->length;++i) {
    hash ^= ( sb->data[i] << ((i % 4) * 8) );
  }
  return hash;
}

Fixnum symbol_table_hash(char const* s) {
  Fixnum hash = 0;
  for(Fixnum i=0; s[i] != 0; ++i) {
    hash ^= ( s[i] << ((i % 4) * 8) );
  }
  return hash;
}

Object* symbol_table_add(SymbolTableBuffer *stb, Object *symbol) {
  if ( stb == 0 ) {
    abort();
  }
  if ( symbol == 0 ) {
    return 0;
  }
 
  Fixnum cand = symbol_table_hash(symbol) % stb->length;
  while( stb->data[cand] != 0 ) {
    if ( string_equals(stb->data[cand], symbol) ) {
      return stb->data[cand];
    }
    cand = (cand + 1) % stb->length;
  }
  stb->data[cand] = symbol;
  stb->occupied++;
  return symbol; 
}

Object* symbol_table_add(SymbolTableBuffer *stb, char const* str) {
	if ( stb == 0 ) {
		abort();
	}
	Fixnum cand = symbol_table_hash(str) % stb->length;
	while( stb->data[cand] != 0 ) {
		if ( string_equals(stb->data[cand], str) ) {
			return stb->data[cand];
		}
		cand = (cand + 1) % stb->length;
	}
	stb->occupied++;
	stb->data[cand] = new_string(str);
	return stb->data[cand];
}

Object *global_symbol_table = new_symbol_table();

Object *sym(char const* str) {
	return symbol_table_add(grow_symbol_table(global_symbol_table), str); 
}

Object *sym(Object *str) {
	return symbol_table_add(grow_symbol_table(global_symbol_table), str); 
}

void add_sym(Object *obj, char const* str) {
	obj->buffer = (Buffer *) new_string_buffer(str);
	symbol_table_add(grow_symbol_table(global_symbol_table), obj);
}

Object *get(Object *target, char const* s) {
  return get(target, sym(s));
}

Fixnum get_fixnum(Object *obj, char const* s) {
  return get_fixnum(obj, sym(s));
}

void set(Object *target, char const* s, Object *value) {
  set(target, sym(s), value);
}

void set(Object *target, char const* s, char const *v) {
  set(target, sym(s), sym(v));
}

void set_fixnum(Object *obj, char const *s, Fixnum value) {
  set_fixnum(obj, sym(s), value);
}

void init_symbol_table_symbols() {
	add_sym(SymbolTable, "SymbolTable");
}
