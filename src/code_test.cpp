#include "code_test.h"
#include "code.h"
#include "symbols.h"
#include "array.h"
#include "char_array.h"
#include "native_call.h"
#include "debug.h"
#include "frame.h"
#include "interp.h"

#include <stdio.h>

fixnum println(cxt_t *cxt, fixnum arg) {
  fprintf(stdout, "%s\n", get_buf(cxt, arg));
  return 0;
}

void test1(cxt_t *cxt, fixnum code) {
  array_push(cxt, code, SYM_TERM);
};

void test2(cxt_t *cxt, fixnum code) {
  array_push(cxt, code, SYM_PUSH);
  array_push(cxt, code, 0);
  array_push(cxt, code, SYM_POP);
  array_push(cxt, code, SYM_TERM);
};

void test3(cxt_t *cxt, fixnum code) {
  array_push(cxt, code, SYM_NEW);    // code block
  array_push(cxt, code, SYM_SPUSH);
  array_push(cxt, code, SYM_RET);
  array_push(cxt, code, SYM_CALL);
  array_push(cxt, code, SYM_TERM);
};

void test4(cxt_t *cxt, fixnum code) {
  array_push(cxt, code, SYM_PUSH);
  array_push(cxt, code, new_char_array(cxt, "Hello World"));
  code_native_call(cxt, code, &println);
  array_push(cxt, code, SYM_TERM);
};

void test5(cxt_t *cxt, fixnum code) {
  fixnum obj = new_array(cxt);
  fixnum parent = new_array(cxt);
  set(cxt, obj, SSYM_PARENT, parent);
  set(cxt, parent, SYM_EXAMPLE, new_char_array(cxt, "Example"));
  code_push(cxt, code, obj);
  code_push(cxt, code, SYM_EXAMPLE);
  code_push(cxt, code, code_send(cxt));
  code_call(cxt, code);
  code_native_call(cxt, code, &println);
  code_term(cxt, code);
};

void execute_code(void (*code_gen)(cxt_t *cxt, fixnum code)) {
  trace("-----");
  cxt_t *cxt = new_cxt();
  fixnum code = new_array(cxt);
  (*code_gen)(cxt, code);
  fixnum frame = new_frame(cxt, code, 0);
  interp(cxt, frame);
}

void code_test_execute_tests() {
  execute_code(&test1);
  execute_code(&test2);
  execute_code(&test3);
  execute_code(&test4);
  execute_code(&test5);
}


