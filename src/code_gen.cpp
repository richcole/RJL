#include "code_gen.h"

void parser_parse(cxt_t *cxt, char const* filename) {
  fixnum file    = new_file(cxt, new_char_array(cxt, filename));
  fixnum scanner = new_scanner(cxt, file);
  fixnum parser  = new_parser(cxt, scanner);
  fixnum ast     = parser_parse(cxt, parser);
  fixnum errors  = set(cxt, parser, SYM_ERRORS);
};


