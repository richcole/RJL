#ifndef SYMBOLS_H
#define SYMBOLS_H

// SSYM symbols are system symbols used in the frame
// object. They shouldn't collide with normal symbols
//
// SYM symbols are normal symbols.

#define SSYM_DIRTY       1
#define SSYM_PC          2
#define SYM_NEW         3
#define SYM_SET_FRAME   4
#define SYM_GET_FRAME   5
#define SYM_SET         6
#define SYM_GET         7
#define SYM_FSET        8
#define SYM_FGET        9
#define SYM_SGET        10
#define SSYM_CODE       11
#define SSYM_STACK      12
#define SYM_SET_PC      13
#define SYM_TERM        14
#define SYM_PUSH        15
#define SYM_SPUSH       16
#define SYM_POP         17
#define SYM_ABORT       18
#define SYM_CALL        19
#define SYM_NATIVE_CALL 20
#define SSYM_RET        21
#define SYM_JMP         22
#define SSYM_PARENT     23
#define SYM_EXAMPLE     24
#define SYM_RETZ        25
#define SYM_RETNZ       26
#define SYM_TARGET      27
#define SYM_SLOT        28
#define SYM_ARG         29
#define SYM_JMPZ        30 
#define SYM_JMPNZ       31 
#define SYM_RET         32

#define SYM_COND        33 
#define SYM_TRUE_BLOCK  34 
#define SYM_FALSE_BLOCK 35
#define SYM_BLOCK       36

#define SYM_FILENAME    37
#define SYM_FILE        38
#define SYM_NEXT_CHAR   39
#define SYM_CURR_CHAR   40
#define SYM_LINE_NUM    41
#define SYM_CHAR_NUM    42

#define SYM_BRACE_LEFT   43
#define SYM_BRACE_RIGHT  44
#define SYM_COMMA        45
#define SYM_EQUALS       46
#define SYM_NUMBER       47
#define SYM_PIPE         48
#define SYM_SEMI         49
#define SYM_TYPE         50
#define SYM_EOF          51
#define SYM_STRING       52
#define SYM_IDENT        53

#endif
