
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
  init_scanner_sys(sys);
  return sys;
}

void code_push(Object *code, Object *val) {
  push(code, Push);
  push(code, val);
}

void code_send(Object *code, Object *slot) {
  push(code, Send);
  push(code, slot);
}

void code_self(Object *code) {
  push(code, Self);
}

void code_return(Object *code) {
  push(code, Return);
}

void code_term(Object *code) {
  push(code, Term);
}

Object *top_level_frame(Object *sys) {
  Object *code       = new_array();
  Object *frame      = new_frame(0, code, 0);
  
  // set the lexical parent to be sys
  set_lexical_parent(frame, sys);

  // main parent code
  code_push(code, new_string("Hello World"));
  code_self(code);
  code_send(code, sym("println:"));
  code_term(code);

  // parent catch block
  push_slot(frame, Catch, object(array_length(code)));
  push(code, Arg);
  push(code, sym("ex"));
  push(code, Push);
  push(code, new_string("Exception raised."));
  push(code, Push);
  push(code, Self);
  code_push(code, sym("println:"));
  push(code, Return);

  return frame;
}

int main() {
  init_symbols();
  Object *sys = init_sys();

  interp(top_level_frame(sys));
};
