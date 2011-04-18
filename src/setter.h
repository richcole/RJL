#ifndef SETTER_H
#define SETTER_H

#include "object.h"

Fixnum  is_nocall_slot(Object *cxt, Object *value);
Object* get_nocall_slot(Object *cxt, Object *slot);
Fixnum  is_setter_slot(Object *cxt, Object *value);
Object* setter_field(Object *cxt, Object *value);

#endif
