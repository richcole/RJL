#include "object.h"
#include "sym.h"
#include "mem.h"
#include "context.h"

void grow(Object *obj);

Object* new_object(Object *cxt) {
  Object *obj = (Object *) mem_alloc(sizeof(Object));
  obj->length = 4;
  obj->occupied = 0;
  obj->table  = (ObjectPair *) mem_alloc(sizeof(ObjectPair)*obj->length);
  obj->buffer = 0;
  if ( cxt != 0 ) {
    set(cxt, obj, "parent", context_get(cxt, "Object"));
  }
  return obj;
};

Fixnum fixnum(Object *obj) {
  Fixnum sign_bit = (Fixnum)obj & (0x1L << ((sizeof(Fixnum) * 8) - 1));
  return ((Fixnum)obj >> 2) | sign_bit;
}

Object *object(Fixnum fixnum) {
    Fixnum sign_bit = (Fixnum)fixnum & (0x1L << ((sizeof(Fixnum) * 8) - 1));
    return (Object *)(((Fixnum)fixnum << 2) | sign_bit | 0x1);
}

bool is_fixnum(Object *obj) {
    return ((Fixnum)obj & 0x1) == 0x1;
}

bool is_object(Object *obj) {
    return obj != 0 && ((Fixnum)obj & 0x3) == 0;
}

Fixnum hash(Object *key, Fixnum length) {
  return ((Fixnum)key >> 2) % length;
}

void set(Object *obj, Object *key, Object *value) {
  if ( obj->occupied * 4 >= obj->length * 3 ) {
    grow(obj);
  }
  Fixnum cand = hash(key, obj->length);
  while(obj->table[cand].key != key && obj->table[cand].key != 0) {
    cand = (cand + 1) % obj->length;
  }
  if ( obj->table[cand].key != key ) {
    obj->occupied++;
  }
  obj->table[cand].key = key;
  obj->table[cand].value = value;
}

Object *get_plain(Object *obj, Object *key) {
  if ( obj == 0 || ! is_object(obj) ) {
    return 0;
  }
  Fixnum cand = hash(key, obj->length);
  while(obj->table[cand].key != key && obj->table[cand].key != 0) {
    cand = (cand + 1) % obj->length;
  }
  if ( obj->table[cand].key == 0 ) {
    return 0;
  }
  else {
    return obj->table[cand].value;
  }
}

void grow(Object *obj) {
  Object tmp;
  tmp.occupied = 0;
  tmp.length = obj->length*2;
  tmp.table = (ObjectPair *) mem_alloc(sizeof(ObjectPair)*tmp.length);
  for(Fixnum i=0;i<obj->length; ++i) {
    if ( obj->table[i].key != 0 ) {
      set(&tmp, obj->table[i].key, obj->table[i].value);
    }
  }
  mem_free(obj->table);
  obj->occupied = tmp.occupied;
  obj->length = tmp.length;
  obj->table = tmp.table;
}

Fixnum exists(Object *cxt, Object *obj) {
  return obj != 0 && obj != get_undefined(cxt);
}

Object *get(Object *cxt, Object *target, Object *slot) {
  Object *value = get_plain(target, slot);
  while ( value == 0 ) {
    Object *new_target = get_plain(target, sym(cxt, "parent"));
    if ( new_target == 0 ) {
      return get_undefined(cxt);
    }
    target = new_target;
    value = get_plain(target, slot);
  }
  return value;
};

void copy(Object *cxt, Object *dst, Object *src, Object *slot) {
  set(dst, slot, get(cxt, src, slot));
}

Object* new_object(Object *cxt, Object *parent) {
  Object *obj = new_object(cxt);
  set(obj, sym(cxt, "parent"), parent);
  return obj;
};

Object* get_parent(Object *cxt, Object *obj) {
  return get(cxt, obj, "parent");
};

Object* new_object(Object *cxt, char const* parent_name) {
  return new_object(cxt, context_get(cxt, parent_name));
}

