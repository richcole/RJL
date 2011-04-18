#ifndef BLOCK_H
#define BLOCK_H

#include "object.h"

Fixnum is_block(Object *cxt, Object *block);
Object* new_block(Object *cxt);

#endif
