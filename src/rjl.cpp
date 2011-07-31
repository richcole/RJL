
#include "std.h"

#include "file.h"
#include "frame.h"
#include "setter.h"
#include "scanner.h"
#include "parser.h"
#include "code_generator.h"
#include "code.h"

#include "interp.h"

Object* dump_parse_error(Object *cxt, Object *code) {
  Object *error_jmp_point = new_boxed_int(cxt, array_length(cxt, code));
  code_self_send(cxt, code, "parse_cxt");
  code_send(cxt, code, "errors");
  code_self_send(cxt, code, "dump:");

  return error_jmp_point;
}

Fixnum parse_file(Object *cxt, Object *code) {
  // main parent code
  code_self_send(cxt, code, "File");
  code_send(cxt, code, "open:");
  code_self_send(cxt, code, "Scanner");
  code_send(cxt, code, "tokenize:");
  code_self_send(cxt, code, "Parser");
  code_send(cxt, code, "parse:");
  code_self_send(cxt, code, "parse_cxt:");

  code_push(cxt, code, new_boxed_int(cxt, 0));
  code_self_send(cxt, code, "parse_cxt");
  code_send(cxt, code, "errors");
  code_send(cxt, code, "length");
  code_send(cxt, code, ">:");

  Fixnum errors_label = code_jmp_true(cxt, code, 0);

  code_self_send(cxt, code, "parse_cxt");
  code_self_send(cxt, code, "CodeGenerator");
  code_send(cxt, code, "generate:");
  code_self_send(cxt, code, "context:");
  code_push(cxt, code, cxt);
  code_self_send(cxt, code, "context");
  code_send(cxt, code, "code:");

  return errors_label;
};

Object *top_level_frame(Object *cxt) {
  Object *code       = new_array(cxt);
  Object *ret_frame  = new_frame(cxt, get_undefined(cxt), code, get_undefined(cxt));
  Object *frame      = new_frame(cxt, get_undefined(cxt), code, ret_frame);
  Object *jmp_point  = 0;
  Fixnum fixup1;
  
  // set the lexical parent to be sys
  set_lexical_parent(cxt, frame, cxt);

  // read the first filename in args
  code_push(cxt, code, new_char_array(cxt, "std/core.r"));
  fixup1 = parse_file(cxt, code);

  // terminate the program
  code_term(cxt, code);

  jmp_point = dump_parse_error(cxt, code);
  set_at(cxt, code, fixup1, jmp_point);

  // catch block
  Object *catch_code = new_array(cxt);
  set(cxt, code, "catch", catch_code);
  code_arg(cxt, catch_code, "ex");
  code_push(cxt, catch_code, new_char_array(cxt, "Exception raised."));
  code_self_send(cxt, catch_code, "println:");
  code_self_send(cxt, catch_code, "ex");
  code_push(cxt, catch_code, new_boxed_int(cxt, 8));
  code_self_send(cxt, catch_code, sym(cxt, "dump:to:"));
  code_term(cxt, catch_code);

  return frame;
}

Object* parse_arguments(Object *cxt, int argc, char **argv) {
  Object *args = new_array(cxt);
  for(int i=1;i<argc; ++i) {
    push(cxt, args, new_char_array(cxt, argv[i]));
  }
  return args;
}

int main(int argc, char **argv) {
  Object *cxt = new_context();
  context_set(cxt, "args", parse_arguments(cxt, argc, argv));
  interp(cxt, top_level_frame(cxt));
  context_mark_and_sweep(cxt, 0);
};
