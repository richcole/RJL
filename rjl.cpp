#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

typedef unsigned char byte_t;
typedef unsigned int  fixnum;

struct obj_t;
struct slot_t;

struct slot_t {
  fixnum slot;
  fixnum value;
};

struct obj_t {
  slot_t* slots;
  fixnum  slots_len; 
  fixnum  num_slots;
  fixnum* array;
  fixnum  array_len;
  fixnum  array_tail;
  byte_t* buf;
  fixnum  buf_len;
  fixnum  buf_tail;
};

struct cxt_t {
  obj_t   *objs;
  fixnum  objs_len;
  fixnum  objs_tail;
};

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
  obj->slots      = (slot_t *)calloc(obj->slots_len, sizeof(slot_t));
  obj->array      = 0;
  obj->array_len  = 0;
  obj->array_tail = 0;
  obj->buf        = 0;
  obj->buf_len    = 0;
  return obj_id;
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

void set(cxt_t *cxt, fixnum obj_id, fixnum slot_id, fixnum value) {
  obj_t *obj = to_obj(cxt, obj_id);
  if ( obj->num_slots * 4 > obj->slots_len * 3 ) {
    abort(); // FIXME: resize
  }
  slot_t *table = obj->slots;
  fixnum slot_index = slot_id % obj->slots_len;
  fixnum curr_slot = table[slot_index].slot;
  while( curr_slot != 0 && curr_slot != slot_id ) {
    slot_index = ( slot_index + 1 ) % obj->slots_len;
    curr_slot = table[slot_index].slot;
  }
  if ( table[slot_index].slot == 0 ) {
    obj->num_slots++;
  }
  table[slot_index].slot = slot_id;
  table[slot_index].value = value;
}
    
// SSYM symbols are system symbols used in the frame
// object. They shouldn't collide with normal symbols
//
// SYM symbols are normal symbols.

#define SSYM_DIRTY       1
#define SSYM_PC          2
#define SYM_NEW         3
#define SYM_SET_FRAME   4
#define SYM_GET_FRAME   5
#define SYM_SET         6
#define SYM_GET         7
#define SYM_FSET        8
#define SYM_FGET        9
#define SYM_SGET        10
#define SSYM_CODE       11
#define SSYM_STACK      12
#define SYM_SET_PC      13
#define SYM_TERM        14
#define SYM_PUSH        15
#define SYM_SPUSH       16
#define SYM_POP         17
#define SYM_ABORT       18
#define SYM_CALL        19
#define SYM_NATIVE_CALL 20
#define SSYM_RET        21
#define SYM_JMP         22
#define SSYM_PARENT     23
#define SYM_EXAMPLE     24
#define SYM_RETZ        25
#define SYM_RETNZ       26
#define SYM_TARGET      27
#define SYM_SLOT        28
#define SYM_ARG         29
#define SYM_JMPZ        30 
#define SYM_JMPNZ       31 
#define SYM_RET         32

#define SYM_COND        32 
#define SYM_TRUE_BLOCK  33 
#define SYM_FALSE_BLOCK 34
#define SYM_BLOCK       35

#define SYM_LENGTH     4294967295

void push(cxt_t *cxt, fixnum obj_id, fixnum value) {
  obj_t *obj = to_obj(cxt, obj_id);
  if ( obj->array == 0 ) {
    obj->array_len = 10;
    obj->array = (fixnum *)calloc(10, sizeof(fixnum));
  }
  else if ( obj->array_tail == obj->array_len ) {
    fixnum new_len = obj->array_len * 2;
    fixnum *new_array = (fixnum *)calloc(new_len, sizeof(fixnum));
    memcpy(new_array, obj->array, sizeof(fixnum)*obj->array_len);
    free(obj->array);
    obj->array = new_array;
    obj->array_len = new_len;
  }
  obj->array[obj->array_tail++] = value;
}

fixnum pop(cxt_t *cxt, fixnum obj_id) {
  obj_t *obj = to_obj(cxt, obj_id);
  if ( obj->array_tail == 0 ) {
    return 0;
  }
  else {
    return obj->array[--obj->array_tail];
  }
}

fixnum get_at(cxt_t *cxt, fixnum obj_id, fixnum pos) {
  obj_t *obj = to_obj(cxt, obj_id);
  if ( obj->array != 0 && pos < obj->array_tail ) {
    return obj->array[pos];
  }
  else {
    return 0;
  }
}

void set_at(cxt_t *cxt, fixnum obj_id, fixnum pos, fixnum value) {
  obj_t *obj = to_obj(cxt, obj_id);
  if ( obj->array != 0 && pos < obj->array_tail ) {
    obj->array[pos] = value;
  }
  else {
    abort(); // FIXME: resize the array
  }
}

fixnum peek(cxt_t *cxt, fixnum obj_id) {
  obj_t *obj = to_obj(cxt, obj_id);
  if ( obj->array_tail == 0 ) {
    return 0;
  }
  else {
    return obj->array[obj->array_tail-1];
  }
}

fixnum array_length(cxt_t *cxt, fixnum obj_id) {
  obj_t *obj = to_obj(cxt, obj_id);
  return obj->array_tail;
}

void incr(cxt_t* cxt, fixnum obj, fixnum slot) {
  set(cxt, obj, slot, get(cxt, obj, slot)+1);
}

void trace(char const* msg) {
  fprintf(stdout, "TRACE %s\n", msg);
};

void dump(cxt_t *cxt, fixnum obj_id) {
  obj_t *obj = to_obj(cxt, obj_id);
  fixnum i;
  for(i=0;i<obj->slots_len;++i) {
    slot_t *slot = obj->slots + i;
    fprintf(stdout, "  %u %u\n", slot->slot, slot->value);
  }
  for(i=0;i<obj->array_len;++i) {
    fprintf(stdout, "  %u\n", obj->array[i]);
  }
  if (obj->buf != 0) {
    fprintf(stdout, "  buf=%p\n", obj->buf);
    fprintf(stdout, "  len=%u\n", obj->buf_len);
  }
}

fixnum new_frame(cxt_t *cxt, fixnum code, fixnum ret) {
  fixnum frame = new_obj(cxt);
  set(cxt, frame, SSYM_PC, 0);
  set(cxt, frame, SSYM_CODE, code);
  set(cxt, frame, SSYM_STACK, new_obj(cxt));
  set(cxt, frame, SSYM_RET, ret);
  return frame;
}
  
fixnum next_instr(cxt_t *cxt, fixnum frame) {
  fixnum code  = get(cxt, frame, SSYM_CODE);
  fixnum pc    = get(cxt, frame, SSYM_PC);
  fixnum instr = get_at(cxt, code, pc);
  incr(cxt, frame, SSYM_PC);
  return instr;
}

fixnum new_string(cxt_t *cxt, char const* s) {
  fixnum obj_id = new_obj(cxt);
  fixnum len = strlen(s)+1;
  obj_t *obj = to_obj(cxt, obj_id);
  obj->buf_len = len;
  obj->buf = (byte_t *) malloc(len);
  memcpy(obj->buf, s, len);
  return obj_id;
}

typedef fixnum (*native_call)(cxt_t *cxt, fixnum arg);

fixnum println(cxt_t *cxt, fixnum arg) {
  obj_t *obj = to_obj(cxt, arg);
  fprintf(stdout, "%s\n", obj->buf);
  return 0;
}

fixnum new_native_call(cxt_t *cxt, native_call call) {
  fixnum obj_id = new_obj(cxt);
  fixnum len = sizeof(native_call);
  obj_t *obj = to_obj(cxt, obj_id);
  obj->buf_len = len;
  obj->buf = (byte_t *) malloc(len);
  memcpy(obj->buf, &call, len);
  return obj_id;
}

native_call get_native_call(cxt_t *cxt, fixnum obj_id)
{
  obj_t *obj = to_obj(cxt, obj_id);
  if ( obj->buf == 0 ) {
    abort();
  }
  return *(native_call*)obj->buf;
}


void interp(cxt_t *cxt, fixnum frame) {

  fixnum obj_id, slot_id, value_id, pc, instr, stack, ret_frame, ret_stack;
  fixnum call_id, arg, target, parent, parent_stack;
  native_call func;
  fixnum finished = 0;
  
  while(! finished) {
    instr = next_instr(cxt, frame);
    stack = get(cxt, frame, SSYM_STACK);
    switch(instr) {
    case SYM_NEW:
      trace("SYM_NEW");
      push(cxt, stack, new_obj(cxt));
      break;
    case SYM_SET_FRAME:
      trace("SYM_SET_FRAME");
      frame = pop(cxt, stack);
      break;
    case SYM_GET_FRAME:
      trace("SYM_GET_FRAME");
      push(cxt, stack, frame);
      break;
    case SYM_ARG:
      trace("SYM_ARG");
      parent = get(cxt, frame, SSYM_RET);
      parent_stack = get(cxt, parent, SSYM_STACK);
      value_id = pop(cxt, parent_stack);
      slot_id = next_instr(cxt, frame);
      set(cxt, frame, slot_id, value_id);
      break;
    case SYM_SET:
      trace("SYM_SET");
      value_id = pop(cxt, stack);
      slot_id = pop(cxt, stack);
      obj_id = pop(cxt, stack);
      set(cxt, obj_id, slot_id, value_id);
      break;
    case SYM_FSET:
      trace("SYM_FSET");
      slot_id = next_instr(cxt, frame);
      set(cxt, frame, slot_id, pop(cxt, stack));
      break;
    case SYM_GET:
      trace("SYM_GET");
      slot_id = pop(cxt, stack);
      obj_id = pop(cxt, stack);
      push(cxt, stack, get(cxt, obj_id, slot_id));
      break;
    case SYM_FGET:
      trace("SYM_FGET");
      slot_id = next_instr(cxt, frame);
      push(cxt, stack, get(cxt, frame, slot_id));
      break;
    case SYM_SGET:
      trace("SYM_SGET");
      target = pop(cxt, stack);
      slot_id = next_instr(cxt, frame);
      push(cxt, stack, get(cxt, target, slot_id));
      break;
    case SYM_PUSH:
      trace("SYM_PUSH");
      push(cxt, stack, next_instr(cxt, frame));
      break;
    case SYM_SPUSH:
      trace("SYM_SPUSH");
      push(cxt, peek(cxt, stack), next_instr(cxt, frame));
      break;
    case SYM_POP:
      trace("SYM_POP");
      pop(cxt, stack);
      break;
    case SYM_SET_PC:
      trace("SYM_SET_PC");
      pc = pop(cxt, stack);
      set(cxt, frame, SSYM_PC, pc);
      break;
    case SYM_JMP:
      trace("SYM_JMP");
      pc = next_instr(cxt, frame);
      set(cxt, frame, SSYM_PC, pc);
      break;
    case SYM_JMPZ:
      trace("SYM_JMP");
      value_id = pop(cxt, stack);
      pc = next_instr(cxt, frame);
      if ( value_id == 0 ) {
        set(cxt, frame, SSYM_PC, pc);
      }
      break;
    case SYM_JMPNZ:
      trace("SYM_JMP");
      value_id = pop(cxt, stack);
      pc = next_instr(cxt, frame);
      if ( value_id != 0 ) {
        set(cxt, frame, SSYM_PC, pc);
      }
      break;
    case SYM_TERM:
      trace("SYM_TERM");
      finished = 1;
      break;
    case SYM_ABORT:
      trace("SYM_ABORT");
      finished = 1;
      break;
    case SYM_CALL:
      trace("SYM_CALL");
      frame = new_frame(cxt, pop(cxt, stack), frame);
      break;
    case SYM_NATIVE_CALL:
      trace("SYM_NATIVE_CALL");
      call_id  = pop(cxt, stack);
      arg      = pop(cxt, stack);
      func     = get_native_call(cxt, call_id);
      value_id = (*func)(cxt, arg);
      push(cxt, stack, value_id);
      break;
    case SYM_RET:
      trace("SYM_RET");
      ret_frame = get(cxt, frame, SSYM_RET);
      ret_stack = get(cxt, ret_frame, SSYM_STACK);
      push(cxt, ret_stack, pop(cxt, stack));
      frame = ret_frame;
      break;
    case SYM_RETZ:
      trace("SYM_RETZ");
      value_id = peek(cxt, stack);
      if ( value_id == 0 ) {
        ret_frame = get(cxt, frame, SSYM_RET);
        ret_stack = get(cxt, ret_frame, SSYM_STACK);
        push(cxt, ret_stack, pop(cxt, stack));
        frame = ret_frame;
      }
      break;
    case SYM_RETNZ:
      trace("SYM_RETNZ");
      value_id = peek(cxt, stack);
      if ( value_id != 0 ) {
        ret_frame = get(cxt, frame, SSYM_RET);
        ret_stack = get(cxt, ret_frame, SSYM_STACK);
        push(cxt, ret_stack, pop(cxt, stack));
        frame = ret_frame;
      }
      break;
    };
  }
}

void code_arg(cxt_t *cxt, fixnum code, fixnum slot) {
  push(cxt, code, SYM_ARG);
  push(cxt, code, slot);
}

void code_fget(cxt_t *cxt, fixnum code, fixnum slot) {
  push(cxt, code, SYM_FGET);
  push(cxt, code, slot);
}

void code_fset(cxt_t *cxt, fixnum code, fixnum slot) {
  push(cxt, code, SYM_FSET);
  push(cxt, code, slot);
}

void code_get(cxt_t *cxt, fixnum code) {
  push(cxt, code, SYM_GET);
}

void code_retnz(cxt_t *cxt, fixnum code) {
  push(cxt, code, SYM_RETNZ);
}

void code_ret(cxt_t *cxt, fixnum code) {
  push(cxt, code, SYM_RET);
}

void code_retz(cxt_t *cxt, fixnum code) {
  push(cxt, code, SYM_RETZ);
}

fixnum code_jmp(cxt_t *cxt, fixnum code, fixnum pos) {
  push(cxt, code, SYM_JMP);
  fixnum label = array_length(cxt, code);
  push(cxt, code, pos);
  return label;
}

fixnum code_jmpz(cxt_t *cxt, fixnum code, fixnum pos) {
  push(cxt, code, SYM_JMPZ);
  fixnum label = array_length(cxt, code);
  push(cxt, code, pos);
  return label;
}

fixnum code_jmpnz(cxt_t *cxt, fixnum code, fixnum pos) {
  push(cxt, code, SYM_JMPNZ);
  fixnum label = array_length(cxt, code);
  push(cxt, code, pos);
  return label;
}

void code_sget(cxt_t *cxt, fixnum code, fixnum slot) {
  push(cxt, code, SYM_SGET);
  push(cxt, code, slot);
}

void code_call(cxt_t *cxt, fixnum code) {
  push(cxt, code, SYM_CALL);
}

void code_push(cxt_t *cxt, fixnum code, fixnum value) {
  push(cxt, code, SYM_PUSH);
  push(cxt, code, value);
}

void code_term(cxt_t *cxt, fixnum code) {
  push(cxt, code, SYM_TERM);
}

fixnum code_send(cxt_t *cxt) {
  fixnum code = new_obj(cxt);
  code_arg(cxt, code, SYM_SLOT);
  code_arg(cxt, code, SYM_TARGET);
  fixnum loop = array_length(cxt, code);
  code_fget(cxt, code, SYM_TARGET);
  code_fget(cxt, code, SYM_SLOT);
  code_get(cxt, code);
  code_retnz(cxt, code);
  code_fget(cxt,  code, SYM_TARGET);
  code_sget(cxt,  code, SSYM_PARENT);
  code_retz(cxt, code);
  code_fset(cxt, code, SYM_TARGET);
  code_jmp(cxt,  code, loop);
  return code;
}

void set_label(cxt_t *cxt, fixnum code, fixnum else_jmp) {
  set_at(cxt, code, else_jmp, array_length(cxt, code));
}

fixnum code_if_else(cxt_t *cxt) {
  fixnum code = new_obj(cxt);
  code_arg(cxt, code, SYM_COND);
  code_arg(cxt, code, SYM_TRUE_BLOCK);
  code_arg(cxt, code, SYM_FALSE_BLOCK);
  code_fget(cxt, code, SYM_COND);
  code_call(cxt, code);
  fixnum else_jmp = code_jmpz(cxt, code, 0);
  code_fget(cxt, code, SYM_TRUE_BLOCK);
  code_call(cxt, code);
  code_ret(cxt, code);
  set_label(cxt, code, else_jmp);
  code_fget(cxt, code, SYM_FALSE_BLOCK);
  code_call(cxt, code);
  code_ret(cxt, code);
  return code;
};

fixnum code_while(cxt_t *cxt) {
  fixnum code = new_obj(cxt);
  code_arg(cxt, code, SYM_COND);
  code_arg(cxt, code, SYM_BLOCK);
  fixnum loop = array_length(cxt, code);
  code_fget(cxt, code, SYM_COND);
  code_call(cxt, code);
  code_retz(cxt, code);
  code_fget(cxt, code, SYM_BLOCK);
  code_call(cxt, code);
  code_jmp(cxt, code, loop);
  return code;
};

void code_native_call(cxt_t* cxt, fixnum code, native_call call) {
  push(cxt, code, SYM_PUSH);
  push(cxt, code, new_native_call(cxt, call));
  push(cxt, code, SYM_NATIVE_CALL);
};

void test1(cxt_t *cxt, fixnum code) {
  push(cxt, code, SYM_TERM);
};

void test2(cxt_t *cxt, fixnum code) {
  push(cxt, code, SYM_PUSH);
  push(cxt, code, 0);
  push(cxt, code, SYM_POP);
  push(cxt, code, SYM_TERM);
};

void test3(cxt_t *cxt, fixnum code) {
  push(cxt, code, SYM_NEW);    // code block
  push(cxt, code, SYM_SPUSH);
  push(cxt, code, SYM_RET);
  push(cxt, code, SYM_CALL);
  push(cxt, code, SYM_TERM);
};

void test4(cxt_t *cxt, fixnum code) {
  push(cxt, code, SYM_PUSH);
  push(cxt, code, new_string(cxt, "Hello World"));
  code_native_call(cxt, code, &println);
  push(cxt, code, SYM_TERM);
};

void test5(cxt_t *cxt, fixnum code) {
  fixnum obj = new_obj(cxt);
  fixnum parent = new_obj(cxt);
  set(cxt, obj, SSYM_PARENT, parent);
  set(cxt, parent, SYM_EXAMPLE, new_string(cxt, "Example"));
  code_push(cxt, code, obj);
  code_push(cxt, code, SYM_EXAMPLE);
  code_push(cxt, code, code_send(cxt));
  code_call(cxt, code);
  code_native_call(cxt, code, &println);
  code_term(cxt, code);
};

void execute_code(void (*code_gen)(cxt_t *cxt, fixnum code)) {
  trace("-----");
  cxt_t *cxt = new_cxt();
  fixnum code = new_obj(cxt);
  (*code_gen)(cxt, code);
  fixnum frame = new_frame(cxt, code, 0);
  interp(cxt, frame);
}

int main(int argc, char **argv) {
  execute_code(&test1);
  execute_code(&test2);
  execute_code(&test3);
  execute_code(&test4);
  execute_code(&test5);
}
