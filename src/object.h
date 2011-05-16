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
  Fixnum type;
};

struct Object {
  Fixnum      length;
  Fixnum      occupied;
  Object*     mark;
  ObjectPair *table;
  Buffer     *buffer;
};

Object* new_object(Object *cxt);
Object* new_object(Object *cxt, Object *parent);
Object* new_object(Object *cxt, char const* parent_name);
Object* new_object_no_register();

void    set(Object *cxt, Object *obj, Object *key, Object *value);
void    unset(Object *cxt, Object *obj, Object *key);
Object *get(Object *cxt, Object *target, Object *slot);
Object *get_plain(Object *target, Object *slot);

Object* get_parent(Object *cxt, Object *obj);
Fixnum  exists(Object *cxt, Object *obj);

void object_dispose(Object *cxt, Object *obj);

extern Object *const DirtyKey;

#endif
