#include <stdlib.h>

#include "interp.h"
#include "array.h"
#include "symbols.h"
#include "native_call.h"
#include "debug.h"
#include "frame.h"

fixnum next_instr(cxt_t *cxt, fixnum frame) {
  fixnum code  = get(cxt, frame, SSYM_CODE);
  fixnum pc    = get(cxt, frame, SSYM_PC);
  fixnum instr = array_get_at(cxt, code, pc);
  incr(cxt, frame, SSYM_PC);
  return instr;
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
      array_push(cxt, stack, new_array(cxt));
      break;
    case SYM_SET_FRAME:
      trace("SYM_SET_FRAME");
      frame = array_pop(cxt, stack);
      break;
    case SYM_GET_FRAME:
      trace("SYM_GET_FRAME");
      array_push(cxt, stack, frame);
      break;
    case SYM_ARG:
      trace("SYM_ARG");
      parent = get(cxt, frame, SSYM_RET);
      parent_stack = get(cxt, parent, SSYM_STACK);
      value_id = array_pop(cxt, parent_stack);
      slot_id = next_instr(cxt, frame);
      set(cxt, frame, slot_id, value_id);
      break;
    case SYM_SET:
      trace("SYM_SET");
      value_id = array_pop(cxt, stack);
      slot_id = array_pop(cxt, stack);
      obj_id = array_pop(cxt, stack);
      set(cxt, obj_id, slot_id, value_id);
      break;
    case SYM_FSET:
      trace("SYM_FSET");
      slot_id = next_instr(cxt, frame);
      set(cxt, frame, slot_id, array_pop(cxt, stack));
      break;
    case SYM_GET:
      trace("SYM_GET");
      slot_id = array_pop(cxt, stack);
      obj_id = array_pop(cxt, stack);
      array_push(cxt, stack, get(cxt, obj_id, slot_id));
      break;
    case SYM_FGET:
      trace("SYM_FGET");
      slot_id = next_instr(cxt, frame);
      array_push(cxt, stack, get(cxt, frame, slot_id));
      break;
    case SYM_SGET:
      trace("SYM_SGET");
      target = array_pop(cxt, stack);
      slot_id = next_instr(cxt, frame);
      array_push(cxt, stack, get(cxt, target, slot_id));
      break;
    case SYM_PUSH:
      trace("SYM_PUSH");
      array_push(cxt, stack, next_instr(cxt, frame));
      break;
    case SYM_SPUSH:
      trace("SYM_SPUSH");
      array_push(cxt, array_peek(cxt, stack), next_instr(cxt, frame));
      break;
    case SYM_POP:
      trace("SYM_POP");
      array_pop(cxt, stack);
      break;
    case SYM_SET_PC:
      trace("SYM_SET_PC");
      pc = array_pop(cxt, stack);
      set(cxt, frame, SSYM_PC, pc);
      break;
    case SYM_JMP:
      trace("SYM_JMP");
      pc = next_instr(cxt, frame);
      set(cxt, frame, SSYM_PC, pc);
      break;
    case SYM_JMPZ:
      trace("SYM_JMP");
      value_id = array_pop(cxt, stack);
      pc = next_instr(cxt, frame);
      if ( value_id == 0 ) {
        set(cxt, frame, SSYM_PC, pc);
      }
      break;
    case SYM_JMPNZ:
      trace("SYM_JMP");
      value_id = array_pop(cxt, stack);
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
      frame = new_frame(cxt, array_pop(cxt, stack), frame);
      break;
    case SYM_NATIVE_CALL:
      trace("SYM_NATIVE_CALL");
      call_id  = array_pop(cxt, stack);
      arg      = array_pop(cxt, stack);
      func     = get_native_call(cxt, call_id);
      value_id = (*func)(cxt, arg);
      array_push(cxt, stack, value_id);
      break;
    case SYM_RET:
      trace("SYM_RET");
      ret_frame = get(cxt, frame, SSYM_RET);
      ret_stack = get(cxt, ret_frame, SSYM_STACK);
      array_push(cxt, ret_stack, array_pop(cxt, stack));
      frame = ret_frame;
      break;
    case SYM_RETZ:
      trace("SYM_RETZ");
      value_id = array_peek(cxt, stack);
      if ( value_id == 0 ) {
        ret_frame = get(cxt, frame, SSYM_RET);
        ret_stack = get(cxt, ret_frame, SSYM_STACK);
        array_push(cxt, ret_stack, array_pop(cxt, stack));
        frame = ret_frame;
      }
      break;
    case SYM_RETNZ:
      trace("SYM_RETNZ");
      value_id = array_peek(cxt, stack);
      if ( value_id != 0 ) {
        ret_frame = get(cxt, frame, SSYM_RET);
        ret_stack = get(cxt, ret_frame, SSYM_STACK);
        array_push(cxt, ret_stack, array_pop(cxt, stack));
        frame = ret_frame;
      }
      break;
    default:
      abort();
    };
  }
}

