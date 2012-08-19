#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>

#include "rjl.h"

struct slot_t {
  fixnum slot;
  fixnum value;
};

struct obj_t {
  slot_t* slots;
  fixnum  slots_len; 
  fixnum  num_slots;
  char* buf;
  fixnum  buf_len;
  fixnum  buf_tail;
};

struct cxt_t {
  obj_t   *objs;
  fixnum  objs_len;
  fixnum  objs_tail;
};

char* cxt_alloc(cxt_t *cxt, fixnum len) {
  char *s = (char *)malloc(len);
  memset(s, 0, len);
  return s;
}

void cxt_free(cxt_t *cxt, char *s) {
  return free(s);
}

cxt_t* new_cxt() {
  cxt_t* cxt     = (cxt_t *)malloc(sizeof(cxt_t));
  cxt->objs_len  = 1000;
  cxt->objs      = (obj_t *)calloc(cxt->objs_len, sizeof(obj_t));
  cxt->objs_tail = 1;
  return cxt;
}

obj_t* to_obj(cxt_t *cxt, fixnum obj_id) {
  if ( obj_id == 0 || obj_id >= cxt->objs_len ) {
    abort();
  }
  return cxt->objs + obj_id;
}

fixnum new_obj(cxt_t *cxt) {
  if (cxt->objs_tail >= cxt->objs_len) {
    abort(); // FIXME
  }
  fixnum obj_id   = cxt->objs_tail++;
  obj_t *obj      = to_obj(cxt, obj_id);
  obj->slots_len  = 10;
  obj->slots      = (slot_t *)cxt_alloc(cxt, obj->slots_len * sizeof(slot_t));
  obj->buf        = 0;
  obj->buf_len    = 0;
  obj->buf_tail   = 0;
  return obj_id;
}

void set_buf_len(cxt_t *cxt, fixnum obj_id, fixnum len) {
  obj_t *obj = to_obj(cxt, obj_id);
  obj->buf_len = len;
}

void set_buf_tail(cxt_t *cxt, fixnum obj_id, fixnum tail) {
  obj_t *obj = to_obj(cxt, obj_id);
  obj->buf_tail = tail;
}

fixnum get(cxt_t *cxt, fixnum obj_id, fixnum slot_id) {
  obj_t *obj = to_obj(cxt, obj_id);
  slot_t *table = obj->slots;
  fixnum slot_index = slot_id % obj->slots_len;
  fixnum curr_slot = table[slot_index].slot;
  while( curr_slot != 0 && curr_slot != slot_id ) {
    slot_index = ( slot_index + 1 ) % obj->slots_len;
    curr_slot = table[slot_index].slot;
  }
  return table[slot_index].value;
}

fixnum table_insert(slot_t *table, fixnum slots_len, fixnum slot_id, fixnum value) {
  fixnum slot_index = slot_id % slots_len;
  fixnum curr_slot = table[slot_index].slot;
  fixnum added_slots = 0;
  while( curr_slot != 0 && curr_slot != slot_id ) {
    slot_index = ( slot_index + 1 ) % slots_len;
    curr_slot = table[slot_index].slot;
  }
  if ( table[slot_index].slot == 0 ) {
    added_slots = 1;
  }
  table[slot_index].slot = slot_id;
  table[slot_index].value = value;
  return added_slots;
};

void reserve(cxt_t *cxt, fixnum obj_id, fixnum new_len) {
  obj_t *obj = to_obj(cxt, obj_id);
  slot_t *new_slots = (slot_t *)cxt_alloc(cxt, new_len * sizeof(slot_t));
  fixnum new_num_slots = 0;
  for(fixnum i=0; i<obj->slots_len; i++) {
    if ( obj->slots[i].slot != 0 && obj->slots[i].value != 0 ) {
      new_num_slots += table_insert(
        new_slots, new_len, obj->slots[i].slot, obj->slots[i].value
      );
    }
  }
  cxt_free(cxt, (char *)obj->slots);
  obj->slots = new_slots;
  obj->slots_len = new_len;
  obj->num_slots = new_num_slots;
}

fixnum set(cxt_t *cxt, fixnum obj_id, fixnum slot_id, fixnum value) {
  obj_t *obj = to_obj(cxt, obj_id);
  if ( obj->num_slots * 4 > obj->slots_len * 3 ) {
    reserve(cxt, obj_id, obj->slots_len*2);
  }
  slot_t *table = obj->slots;
  obj->num_slots += table_insert(table, obj->slots_len, slot_id, value);
  return obj_id;
}

void incr(cxt_t* cxt, fixnum obj, fixnum slot) {
  set(cxt, obj, slot, get(cxt, obj, slot)+1);
}

void set_buf(cxt_t *cxt, fixnum obj_id, char const* buf, fixnum buf_len) {
  obj_t *obj = to_obj(cxt, obj_id);
  obj->buf_len = buf_len;
  obj->buf_tail = buf_len;
  obj->buf = (char *) malloc(buf_len);
  memcpy(obj->buf, buf, buf_len);
}

void buf_reserve(cxt_t *cxt, fixnum obj, fixnum len) {
  if ( len > get_buf_len(cxt, obj) ) {
    char *new_buf = cxt_alloc(cxt, len);
    memcpy(new_buf, get_buf(cxt, obj), get_buf_tail(cxt, obj));
    cxt_free(cxt, get_buf(cxt, obj));
    fixnum buf_tail = get_buf_tail(cxt, obj);
    set_buf(cxt, obj, new_buf, len);
    set_buf_tail(cxt, obj, buf_tail);
  }
}

void set_buf(cxt_t *cxt, fixnum obj_id, char const* buf, fixnum buf_len, fixnum buf_tail) {
  obj_t *obj = to_obj(cxt, obj_id);
  obj->buf_len = buf_len;
  obj->buf_tail = buf_tail;
  obj->buf = (char *) malloc(buf_len);
  memcpy(obj->buf, buf, buf_len);
}

char* get_buf(cxt_t *cxt, fixnum obj_id) {
  obj_t *obj = to_obj(cxt, obj_id);
  return obj->buf;
}

fixnum get_buf_len(cxt_t *cxt, fixnum obj_id) {
  obj_t *obj = to_obj(cxt, obj_id);
  return obj->buf_len;
}

fixnum get_buf_tail(cxt_t *cxt, fixnum obj_id) {
  obj_t *obj = to_obj(cxt, obj_id);
  return obj->buf_tail;
}

void dump(cxt_t *cxt, fixnum obj_id) {
  obj_t *obj = to_obj(cxt, obj_id);
  fixnum i;
  for(i=0;i<obj->slots_len;++i) {
    slot_t *slot = obj->slots + i;
    fprintf(stdout, "  %u %u\n", slot->slot, slot->value);
  }
  if (obj->buf != 0) {
    fprintf(stdout, "  buf=%p\n", obj->buf);
    fprintf(stdout, "  len=%u\n", obj->buf_len);
  }
}
