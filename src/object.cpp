#include "object.h"
#include "sym.h"
#include "mem.h"
#include "context.h"
#include "abort.h"

Object *const DirtyKey = (Object *)0x1;

void grow(Object *cxt, Object *obj);

Object* new_object_no_register() {
  Object *obj = (Object *) context_alloc_buffer(0, sizeof(Object));
  obj->length = 4;
  obj->occupied = 0;
  obj->flags = 0;
  obj->table  = (ObjectPair *) context_alloc_buffer(0, sizeof(ObjectPair)*obj->length);
  obj->buffer = 0;
  return obj;
};

Object* new_object(Object *cxt) {
  Object *obj = new_object_no_register();
  context_register_object(cxt, obj);
  return obj;
};

Fixnum hash(Object *key, Fixnum length) {
  return ((Fixnum)key >> 2) % length;
}

void set(Object *cxt, Object *obj, Object *key, Object *value) {
  if ( obj->occupied * 4 >= obj->length * 3 ) {
    grow(cxt, obj);
  }
  Fixnum cand = hash(key, obj->length);
  Fixnum dirty_cand = -1;
  Object *cand_key = obj->table[cand].key;
  while(cand_key != key && cand_key != 0) {
    if ( cand_key == DirtyKey && dirty_cand == -1 ) {
      dirty_cand = cand;
    }
    cand = (cand + 1) % obj->length;
    cand_key = obj->table[cand].key;
  }
  if ( cand_key != key ) {
    obj->occupied++;
    if ( dirty_cand != -1 ) {
      cand = dirty_cand;
    }
  }
  obj->table[cand].key = key;
  obj->table[cand].value = value;
}

void unset(Object *cxt, Object *obj, Object *key) {
  Fixnum cand = hash(key, obj->length);
  while(obj->table[cand].key != key && obj->table[cand].key != 0 ) {
    cand = (cand + 1) % obj->length;
  }
  if ( obj->table[cand].key == key ) {
    obj->table[cand].key = DirtyKey;
    obj->table[cand].value = 0;
  }
}

Object *get_plain(Object *obj, Object *key) {
  if ( obj == 0  ) {
    abort();
  }
  Fixnum cand = hash(key, obj->length);
  while(obj->table[cand].key != key && obj->table[cand].key != 0) {
    cand = (cand + 1) % obj->length;
  }
  if (obj->table[cand].key == 0) {
    return 0;
  }
  else {
    return obj->table[cand].value;
  }
}

void grow(Object *cxt, Object *obj) {
  Object tmp;
  tmp.occupied = 0;
  tmp.length = obj->length*2;
  tmp.table = (ObjectPair *) context_alloc_buffer(cxt, sizeof(ObjectPair)*tmp.length);
  for(Fixnum i=0;i<obj->length; ++i) {
    if ( obj->table[i].key != 0 && obj->table[i].key != DirtyKey ) {
      set(cxt, &tmp, obj->table[i].key, obj->table[i].value);
    }
  }
  context_free_buffer(cxt, obj->table);
  obj->occupied = tmp.occupied;
  obj->flags = tmp.flags;
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
  set(cxt, dst, slot, get(cxt, src, slot));
}

Object* new_object(Object *cxt, Object *parent) {
  Object *obj = new_object(cxt);
  set(cxt, obj, sym(cxt, "parent"), parent);
  return obj;
};

Object* get_parent(Object *cxt, Object *obj) {
  return get(cxt, obj, "parent");
};

Object* new_object(Object *cxt, char const* parent_name) {
  return new_object(cxt, context_get(cxt, parent_name));
}

