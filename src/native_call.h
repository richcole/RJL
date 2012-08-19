#ifndef NATIVE_CALL_H
#define NATIVE_CALL_H

#include "rjl.h"

typedef fixnum (*native_call)(cxt_t *cxt, fixnum arg);

fixnum new_native_call(cxt_t *cxt, native_call call);
native_call get_native_call(cxt_t *cxt, fixnum obj);

fixnum println(cxt_t *cxt, fixnum arg);

#endif
