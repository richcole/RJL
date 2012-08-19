#include "symbols.h"
#include "char_array.h"

cxt_t   *global_cxt;
fixnum  global_sym_table;

defn_sym(SSYM_DIRTY);
defn_sym(SSYM_PC);
defn_sym(SYM_NEW);
defn_sym(SYM_SET_FRAME);
defn_sym(SYM_GET_FRAME);
defn_sym(SYM_SET);
defn_sym(SYM_GET);
defn_sym(SYM_FSET);
defn_sym(SYM_FGET);
defn_sym(SYM_SGET);
defn_sym(SSYM_CODE);
defn_sym(SSYM_STACK);
defn_sym(SYM_SET_PC);
defn_sym(SYM_TERM);
defn_sym(SYM_PUSH);
defn_sym(SYM_SPUSH);
defn_sym(SYM_POP);
defn_sym(SYM_ABORT);
defn_sym(SYM_CALL);
defn_sym(SYM_NATIVE_CALL);
defn_sym(SSYM_RET);
defn_sym(SYM_JMP);
defn_sym(SSYM_PARENT);
defn_sym(SYM_EXAMPLE);
defn_sym(SYM_RETZ);
defn_sym(SYM_RETNZ);
defn_sym(SYM_TARGET);
defn_sym(SYM_SLOT);
defn_sym(SYM_ARG);
defn_sym(SYM_JMPZ);
defn_sym(SYM_JMPNZ);
defn_sym(SYM_RET);

defn_sym(SYM_COND);
defn_sym(SYM_TRUE_BLOCK);
defn_sym(SYM_FALSE_BLOCK);
defn_sym(SYM_BLOCK);

defn_sym(SYM_FILENAME);
defn_sym(SYM_FILE);
defn_sym(SYM_NEXT_CHAR);
defn_sym(SYM_CURR_CHAR);
defn_sym(SYM_LINE_NUM);
defn_sym(SYM_CHAR_NUM);

defn_sym(SYM_BRACE_LEFT);
defn_sym(SYM_BRACE_RIGHT);
defn_sym(SYM_COMMA);
defn_sym(SYM_EQUALS);
defn_sym(SYM_NUMBER);
defn_sym(SYM_PIPE);
defn_sym(SYM_SEMI);
defn_sym(SYM_TYPE);
defn_sym(SYM_EOF);
defn_sym(SYM_STRING);
defn_sym(SYM_IDENT);

fixnum next_sym(char const* s) {
  if ( global_cxt == 0 ) {
    global_cxt = new_cxt();
  }
  if ( global_sym_table == 0 ) {
    global_sym_table = new_obj(global_cxt);
  }
  fixnum result = new_char_array(global_cxt, s);
  set(global_cxt, global_sym_table, result, result);
  return result;
}

char const* get_sym_buf(fixnum sym) {
  return char_array_get_buf(global_cxt, sym);
}
