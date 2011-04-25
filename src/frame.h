#ifndef FRAME_H
#define FRAME_H

#include "object.h"

Object* new_closure(Object *cxt, Object *block, Object *local, Object *self);
void set_lexical_parent(Object *cxt, Object *frame, Object *parent);
Object* get_lexical_parent(Object *cxt, Object *frame);
Object* new_local(Object *cxt);
Object* new_frame(Object *cxt);
Object* new_frame(Object *cxt, Object *self, Object *code, Object *ret_frame);
Object* get_code(Object *cxt, Object *frame, Fixnum index);
Fixnum get_code_fixnum(Object *cxt, Object *frame, Fixnum index);
Object *get_self(Object *cxt, Object *frame);
Object *get_stack(Object *cxt, Object *frame);





#endif
