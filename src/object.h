#ifndef OBJECT_H
#define OBJECT_H

#include "fixnum.h"
#include "buffer.h"

struct ObjectPair;
struct Object;
struct Buffer;

struct ObjectPair {
  Object *key;
  Object *value;
};

struct Buffer {
  Object     *type;
};

struct Object {
  Fixnum      length;
  Fixnum      occupied;
  ObjectPair *table;
  Buffer     *buffer;
};

Object* new_object(Object *cxt);
Object* new_object(Object *cxt, Object *parent);
Object* new_object(Object *cxt, char const* parent_name);

void    set(Object *obj, Object *key, Object *value);
Object *get(Object *cxt, Object *target, Object *slot);
Object *get_plain(Object *target, Object *slot);

Object* get_parent(Object *cxt, Object *obj);
Fixnum  exists(Object *cxt, Object *obj);

Fixnum  has_buffer_type(Object *cxt, Object *obj, char const* slot);

#endif
