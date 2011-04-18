#ifndef SYM_H
#define SYM_H

#include "object.h"

Object *sym(Object *cxt, char const* str);
Object *sym(Object *cxt, Object *str);
void    set(Object *cxt, Object *obj, char const* slot, Object *value);
void    set(Object *cxt, Object *obj, char const* slot, char const* value);
Object *get(Object *cxt, Object *target, char const* s);

Fixnum  equals_sym(Object *cxt, Object *obj, char const* sym);

Object *get_undefined(Object *cxt);
Object *get_true(Object *cxt);
Object *get_false(Object *cxt);

void    push_undefined(Object *cxt, Object *stack);
void    set_undefined(Object *cxt, Object *obj, char const* slot);
Object* return_undefined(Object *cxt, Object *frame);
Object* return_value(Object *cxt, Object *frame, Object *value);

Fixnum  is_true(Object *cxt, Object *obj);
Fixnum  is_false(Object *cxt, Object *obj);
Fixnum  is_undefined(Object *cxt, Object *obj);
void    set_true(Object *cxt, Object *obj, Object *slot);
void    set_true(Object *cxt, Object *obj, char const* sym);
Fixnum  is_slot_true(Object *cxt, Object *obj, char const* sym);

void    push(Object *cxt, Object *stack, char const* sym);



#endif
