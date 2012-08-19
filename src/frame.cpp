#include "frame.h"
#include "symbols.h"
#include "array.h"

fixnum new_frame(cxt_t *cxt, fixnum code, fixnum ret) {
  fixnum frame = new_array(cxt);
  set(cxt, frame, SSYM_PC, 0);
  set(cxt, frame, SSYM_CODE, code);
  set(cxt, frame, SSYM_STACK, new_array(cxt));
  set(cxt, frame, SSYM_RET, ret);
  return frame;
}
  
