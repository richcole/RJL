#ifndef ARRAY_H
#define ARRAY_H

#include "rjl.h"

fixnum new_array(cxt_t *cxt);
fixnum array_get_at(cxt_t *cxt, fixnum obj_id, fixnum pos);
void array_set_at(cxt_t *cxt, fixnum obj_id, fixnum pos, fixnum value);
void array_push(cxt_t *cxt, fixnum obj_id, fixnum value);
fixnum array_pop(cxt_t *cxt, fixnum obj_id);
fixnum array_peek(cxt_t *cxt, fixnum obj_id);
fixnum array_length(cxt_t *cxt, fixnum obj_id);
void array_reserve(cxt_t* cxt, fixnum obj, fixnum len);

#endif
