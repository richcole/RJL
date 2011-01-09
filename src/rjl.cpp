
#include "fixnum.h"
#include "mem.h"
#include "object.h"
#include "buffer.h"
#include "string.h"
#include "symbol_table.h"
#include "array.h"
#include "symbol.h"
#include "exception.h"
#include "frame.h"
#include "func.h"
#include "block.h"
#include "native.h"

#include "file.h"
#include "scanner.h"
#include "parser.h"

#include "interp.h"

void init_symbols() {
  init_string_symbols();
  init_symbol_table_symbols();
  init_array_symbols();	
  init_func_symbols();
  init_file_symbols();
  init_exception_symbols();
  init_general_symbols();
  init_scanner_symbols();
}

Object* init_sys() {
  Object *sys = new_object();
	init_file_sys(sys);
  init_native_sys(sys);
  return sys;
}

int main() {
  init_symbols();
  Object *sys = init_sys();

  Object *lexical_parent = new_object();
  Object *local = new_object();
  set(lexical_parent, Local, local);

  Object *code  = new_array();
  Object *block = new_block(lexical_parent, code);

  Object *parent_code  = new_array();
  Object *parent_block = new_block(lexical_parent, parent_code);
  Object *parent_frame = new_frame(0, parent_block, 0);

  push(parent_code, Ret);

  Object *frame = new_frame(0, block, parent_frame);

  Object *parent_stack = get(parent_frame, Stack);
  push(parent_stack, sys);

  push(code, Arg);
  push(code, Sys);

  push(code, Push);
  push(code, new_string("Hello World"));

  push(code, Self);
  push(code, Send);
  push(code, Sys);

  push(code, Send);
  push(code, Print);

  push(code, Ret);

  interp(frame);
};
