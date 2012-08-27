#ifndef CHAR_ARRAY_H

#include "rjl.h"

fixnum new_char_array(cxt_t *cxt, char const* s);
fixnum new_char_array(cxt_t *cxt);
fixnum char_array_get_at(cxt_t *cxt, fixnum string, fixnum index);
fixnum char_array_len(cxt_t *cxt, fixnum string);
void char_array_put_char(cxt_t *cxt, fixnum string, fixnum c);
char* char_array_get_buf(cxt_t *cxt, fixnum string);
fixnum char_array_append(cxt_t *cxt, fixnum a, fixnum b);

#endif
