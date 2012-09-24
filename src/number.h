#ifndef NUMBER_H
#define NUMBER_H

#include "rjl.h"

fixnum new_number(cxt_t *cxt, fixnum num);
fixnum number_plus(cxt_t *cxt, fixnum a, fixnum b);
fixnum number_minus(cxt_t *cxt, fixnum a, fixnum b);
fixnum number_equals(cxt_t *cxt, fixnum a, fixnum b);


#endif
