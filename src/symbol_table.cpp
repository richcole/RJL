#include "symbol_table.h"
#include "std.h"
#include "abort.h"
#include "char_array.h"
#include "context.h"

SymbolTableBuffer *new_symbol_table_buffer(Object *cxt, int len) {
  SymbolTableBuffer *buf = (SymbolTableBuffer *) 
    context_alloc_buffer(cxt, sizeof(SymbolTableBuffer)+(len*sizeof(Object *)));
  buf->type     = SymbolTableTypeTag;
  buf->length   = len;
  buf->occupied = 0;
  return buf;
}

Object* new_symbol_table_no_register() {
  Object *symbol_table = new_object_no_register();
  symbol_table->buffer = (Buffer *) new_symbol_table_buffer(0, 10);
  return symbol_table;
}

Object* new_symbol_table(Object *cxt) {
  Object *symbol_table = new_object(cxt);
  symbol_table->buffer = (Buffer *) new_symbol_table_buffer(cxt, 10);
  return symbol_table;
}

def_get_buffer(SymbolTable, symbol_table, SymbolTableTypeTag);

SymbolTableBuffer *grow_symbol_table(Object *cxt, Object *symbol_table) {
  SymbolTableBuffer *stb = get_symbol_table_buffer(symbol_table);
  if ( stb != 0 && stb->occupied * 4 > stb->length * 3 ) {
    SymbolTableBuffer *new_buffer = new_symbol_table_buffer(cxt, stb->length*2);
    for(Fixnum i=0;i<stb->length;++i) {
      symbol_table_add(new_buffer, stb->data[i]);
    }
    context_free_buffer(cxt, stb);
    stb = new_buffer;
    symbol_table->buffer = (Buffer *) stb;
  }
  return stb;
}

Fixnum symbol_table_hash(Object *symbol) {
  CharArrayBuffer *sb = get_char_array_buffer(symbol);
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
    if ( char_array_equals(stb->data[cand], symbol) ) {
      return stb->data[cand];
    }
    cand = (cand + 1) % stb->length;
  }
  stb->data[cand] = symbol;
  stb->occupied++;
  return symbol; 
}

Object* symbol_table_add(Object *cxt, SymbolTableBuffer *stb, char const* str) {
  if ( stb == 0 ) {
    abort();
  }
  Fixnum cand = symbol_table_hash(str) % stb->length;
  while( stb->data[cand] != 0 ) {
    if ( char_array_equals(stb->data[cand], str) ) {
      return stb->data[cand];
    }
    cand = (cand + 1) % stb->length;
  }
  Object *sym = new_char_array(cxt, str);
  stb->occupied++;
  stb->data[cand] = sym;
  return sym;
}

void add_sym(Object *cxt, Object *sym)
{
  symbol_table_add(grow_symbol_table(cxt, cxt), sym); 
};

void add_sym(Object *cxt, Object *obj, char const* str) {
  obj->buffer = (Buffer *) new_char_array_buffer(cxt, str);
  symbol_table_add(grow_symbol_table(cxt, get_symbol_table(cxt)), obj);
}

