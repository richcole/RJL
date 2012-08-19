#ifndef CHAR_ARRAY_H

#include "rjl.h"

fixnum new_char_array(cxt_t *cxt, char const* s);
fixnum new_char_array(cxt_t *cxt);
fixnum char_array_get_at(cxt_t *cxt, fixnum string, fixnum index);
void char_array_put_char(cxt_t *cxt, fixnum string, fixnum c);
char* char_array_get_buf(cxt_t *cxt, fixnum string);

#endif
