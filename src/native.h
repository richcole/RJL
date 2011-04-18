#ifndef NATIVE_H
#define NATIVE_H

#include "object.h"

Object *native_call(Object *cxt, Object *self, Object *slot);
Object *native_call(Object *cxt, Object *self, char const* s);
Object *call_func(Object *cxt, Object *target, Object *func, Object *frame);
Object *native_call(Object *cxt, Object *self, Object *slot, Object *arg1);
Object *native_call(Object *cxt, Object *self, char const* slot, Object *arg1);
Object *native_call(Object *cxt, Object *self, char const* slot, char const* arg1);
Object *native_call(Object *cxt, Object *self, Object *slot, Object *arg1, 
                    Object *arg2);
Object *native_call(Object *cxt, Object *self, Object *slot, Object *arg1, 
                    Object *arg2, Object *arg3);
Object *native_call(Object *cxt, Object *self, char const* slot, Object *arg1, 
                    Object *arg2, Object *arg3);
void init_native_sys(Object *cxt);


#endif

