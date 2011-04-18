#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "object.h"

struct SymbolTableBuffer {
  Fixnum  type;
  Fixnum  length;
  Fixnum  occupied;
  Object  *data[0];
};

Object* new_symbol_table(Object *cxt);
SymbolTableBuffer *grow_symbol_table(Object *symbol_table);
Object* symbol_table_add(SymbolTableBuffer *stb, Object *symbol);
Object* symbol_table_add(SymbolTableBuffer *stb, char const* str);

#endif

