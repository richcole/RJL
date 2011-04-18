
#include "std.h"

#include "file.h"
#include "frame.h"
#include "setter.h"
#include "scanner.h"
#include "parser.h"
#include "code_generator.h"
#include "code.h"

#include "interp.h"

Object *top_level_frame(Object *cxt) {
  Object *code       = new_array(cxt);
  Object *frame      = new_frame(cxt, get_undefined(cxt), code, get_undefined(cxt));
  
  // set the lexical parent to be sys
  set_lexical_parent(cxt, frame, cxt);

  // main parent code
  code_self_send(cxt, code, "args");
  code_send(cxt, code, "pop");
  code_self_send(cxt, code, "file");
  code_send(cxt, code, "open:");
  code_self_send(cxt, code, "Scanner");
  code_send(cxt, code, "tokenize:");
  code_self_send(cxt, code, "Parser");
  code_send(cxt, code, "parse:");
  code_self_send(cxt, code, "CodeGenerator");
  code_send(cxt, code, "generate:");
  code_self_send(cxt, code, "context:");
  code_push(cxt, code, cxt);
  code_self_send(cxt, code, "context");
  code_send(cxt, code, "code:");
  code_term(cxt, code);

  // parent catch block
  push_slot(cxt, frame, "catch", new_boxed_int(cxt, array_length(cxt, code)));
  code_arg(cxt, code, "ex");
  code_push(cxt, code, new_char_array(cxt, "Exception raised."));
  code_self_send(cxt, code, "println:");
  code_self_send(cxt, code, "ex");
  code_send(cxt, code, "reason");
  code_self_send(cxt, code, sym(cxt, "println:"));
  code_return(cxt, code);

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
};
