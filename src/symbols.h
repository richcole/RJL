#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "rjl.h"
#include "char_array.h"

// SSYM symbols are system symbols used in the frame
// object. They shouldn't collide with normal symbols
//
// SYM symbols are normal symbols.


fixnum next_sym(char const* s);
char const* get_sym_buf(fixnum sym);    

#define decl_sym(x) extern const fixnum x
#define defn_sym(x) const fixnum x = next_sym(#x)

decl_sym(SSYM_DIRTY);
decl_sym(SSYM_PC);
decl_sym(SYM_NEW);
decl_sym(SYM_SET_FRAME);
decl_sym(SYM_GET_FRAME);
decl_sym(SYM_SET);
decl_sym(SYM_GET);
decl_sym(SYM_FSET);
decl_sym(SYM_FGET);
decl_sym(SYM_SGET);
decl_sym(SSYM_CODE);
decl_sym(SSYM_STACK);
decl_sym(SYM_SET_PC);
decl_sym(SYM_TERM);
decl_sym(SYM_PUSH);
decl_sym(SYM_SPUSH);
decl_sym(SYM_POP);
decl_sym(SYM_ABORT);
decl_sym(SYM_CALL);
decl_sym(SYM_NATIVE_CALL);
decl_sym(SSYM_RET);
decl_sym(SYM_JMP);
decl_sym(SSYM_PARENT);
decl_sym(SYM_EXAMPLE);
decl_sym(SYM_RETZ);
decl_sym(SYM_RETNZ);
decl_sym(SYM_TARGET);
decl_sym(SYM_SLOT);
decl_sym(SYM_ARG);
decl_sym(SYM_JMPZ);
decl_sym(SYM_JMPNZ);
decl_sym(SYM_RET);

decl_sym(SYM_COND);
decl_sym(SYM_TRUE_BLOCK);
decl_sym(SYM_FALSE_BLOCK);
decl_sym(SYM_BLOCK);

decl_sym(SYM_FILENAME);
decl_sym(SYM_FILE);
decl_sym(SYM_NEXT_CHAR);
decl_sym(SYM_CURR_CHAR);
decl_sym(SYM_LINE_NUM);
decl_sym(SYM_CHAR_NUM);

decl_sym(SYM_BRACE_LEFT);
decl_sym(SYM_BRACE_RIGHT);
decl_sym(SYM_COMMA);
decl_sym(SYM_EQUALS);
decl_sym(SYM_NUMBER);
decl_sym(SYM_PIPE);
decl_sym(SYM_SEMI);
decl_sym(SYM_TYPE);
decl_sym(SYM_EOF);
decl_sym(SYM_STRING);
decl_sym(SYM_IDENT);

decl_sym(SYM_CURR_TOKEN);
decl_sym(SYM_NEXT_TOKEN);
decl_sym(SYM_SCANNER);
decl_sym(SYM_ERROR);
decl_sym(SYM_ERRORS);
decl_sym(SYM_TEXT);
decl_sym(SYM_TOKEN);

decl_sym(SYM_ARGS);
decl_sym(SYM_STMTS);
decl_sym(SYM_CALL_CHAIN);
decl_sym(SYM_PAREN_LEFT);
decl_sym(SYM_PAREN_RIGHT);

decl_sym(SYM_PERIOD);
decl_sym(SYM_LHS);
decl_sym(SYM_RHS);
decl_sym(SYM_NEXT);
#endif
