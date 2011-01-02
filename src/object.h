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

Object* new_object() {
  Object *obj = (Object *) mem_alloc(sizeof(Object));
  obj->length = 4;
  obj->occupied = 0;
  obj->table  = (ObjectPair *) mem_alloc(sizeof(ObjectPair)*obj->length);
  obj->buffer = 0;
  return obj;
};

Object *Dirty = new_object();
Object *Nil = new_object();

void grow(Object *obj);

Fixnum hash(Object *key, Fixnum length) {
  return ((Fixnum)key >> 2) % length;
}

void set(Object *obj, Object *key, Object *value) {
  if ( obj->occupied * 4 > obj->length * 3 ) {
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

Object *get(Object *obj, Object *key) {
  if ( obj == 0 ) {
    return 0;
  }
  Fixnum cand = hash(key, obj->length);
  while(obj->table[cand].key != key && obj->table[cand].key != 0) {
    cand = (cand + 1) % obj->length;
  }
  if ( obj->table[cand].key == 0 ) {
    return Nil;
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
    if ( obj->table[i].key != 0 && obj->table[i].key != Dirty ) {
      set(&tmp, obj->table[i].key, obj->table[i].value);
    }
  }
  mem_free(obj->table);
  obj->occupied = tmp.occupied;
  obj->length = tmp.length;
  obj->table = tmp.table;
}

void set_fixnum(Object *obj, Object *slot, Fixnum value) {
  set(obj, slot, object(value));
}

Fixnum get_fixnum(Object *obj, Object *slot) {
  return fixnum(get(obj, slot));
}


