
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
#include "setter.h"
#include "scanner.h"
#include "parser.h"
#include "foreach.h"
#include "code.h"
#include "code_generator.h"

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
  init_parser_symbols();
  init_code_generator_symbols();
}

Object* init_sys() {
  Object *sys = new_object();
  init_file_sys(sys);
  init_native_sys(sys);
  init_scanner_sys(sys);
  init_parser_sys(sys);
  init_code_generator_sys(sys);
  return sys;
}

Object *top_level_frame(Object *sys) {
  Object *code       = new_array();
  Object *frame      = new_frame(0, code, 0);
  
  // set the lexical parent to be sys
  set_lexical_parent(frame, sys);

  // main parent code
  code_self_send(code, sym("args"));
  code_send(code, sym("pop"));
  code_self_send(code, File);
  code_send(code, sym("open:"));
  code_self_send(code, sym("Scanner"));
  code_send(code, sym("tokenize:"));
  code_self_send(code, sym("Parser"));
  code_send(code, sym("parse:"));
  code_self_send(code, sym("CodeGenerator"));
  code_send(code, sym("generate:"));
  code_self_send(code, sym("context:"));
  code_self_send(code, sym("context"));
  code_self_send(code, sym("dump:"));
  code_push(code, sys);
  code_self_send(code, sym("context"));
  code_send(code, sym("code:"));
  code_term(code);

  // parent catch block
  push_slot(frame, Catch, object(array_length(code)));
  code_arg(code, sym("ex"));
  code_push(code, new_string("Exception raised."));
  code_self_send(code, sym("println:"));
  code_self_send(code, sym("ex"));
  code_send(code, Reason);
  code_self_send(code, sym("println:"));
  push(code, Return);

  return frame;
}

Object* parse_arguments(int argc, char **argv) {
  Object *args = new_array();
  for(int i=1;i<argc; ++i) {
    push(args, new_string(argv[i]));
  }
  return args;
}

int main(int argc, char **argv) {
  init_symbols();
  Object *sys = init_sys();
  set(sys, sym("args"), parse_arguments(argc, argv));

  interp(top_level_frame(sys));
};
