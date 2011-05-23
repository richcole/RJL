#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "object.h"

Object *new_exception(Object *cxt, Object *frame, char const* reason);
Object *new_exception(Object *cxt, Object *frame, Object *reason);
Fixnum is_exception(Object *cxt, Object *ex);
Object *new_exception_frame(Object *cxt, Object *frame, Object *ex);
Object *get_catch_pc(Object *cxt, Object *frame);
void init_exception_sys(Object *cxt);

#endif

