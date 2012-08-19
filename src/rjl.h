#ifndef RJL_H
#define RJL_H

typedef int  fixnum;

struct cxt_t;
struct obj_t;
struct slot_t;

cxt_t* new_cxt();
fixnum new_obj(cxt_t *cxt);
fixnum get(cxt_t *cxt, fixnum obj_id, fixnum slot_id);
fixnum set(cxt_t *cxt, fixnum obj_id, fixnum slot_id, fixnum value);
void incr(cxt_t* cxt, fixnum obj, fixnum slot);
void set_buf(cxt_t *cxt, fixnum obj, char const* buf, fixnum buf_len);
void set_buf(cxt_t *cxt, fixnum obj, char const* buf, fixnum buf_len, 
  fixnum buf_tail);
void set_buf_tail(cxt_t *cxt, fixnum obj, fixnum buf_tail);
char* get_buf(cxt_t *cxt, fixnum obj);
fixnum get_buf_tail(cxt_t *cxt, fixnum obj);
fixnum get_buf_len(cxt_t *cxt, fixnum obj);
void buf_reserve(cxt_t *cxt, fixnum obj, fixnum len);

#endif
