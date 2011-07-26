#include "native.h"
#include "boxed_int.h"
#include "exception.h"
#include "frame.h"
#include "array.h"
#include "char_array.h"
#include "sym.h"
#include "mem.h"
#include "func.h"
#include "block.h"
#include "code.h"
#include "context.h"

#include <stdio.h>

Object *to_long(Object *cxt, Object *frame, Object *obj, long *num) {
  BoxedIntBuffer *obj_buf  = get_boxed_int_buffer(obj);
  if ( obj_buf != 0 ) {
    *num = obj_buf->value;
    return 0;
  }
  else {
    return new_exception(cxt, frame, "expected fixnum or boxed_int");
  }
}

#define TO_LONG(cxt, frame, var, value)               \
  long var;                                           \
  {                                                   \
    Object *ex = to_long(cxt, frame, value, &var);    \
    if ( ex != 0 ) {                                  \
      return ex;                                      \
    };                                                \
  };                                                  \

#define INT_INT_OP(cxt, frame, self, op)                          \
  Object *stack = get_stack(cxt, frame);                          \
  TO_LONG(cxt, frame, self_long,  self);                          \
  TO_LONG(cxt, frame, other_long, pop(cxt, stack));               \
  push(cxt, stack, new_boxed_int(cxt, self_long op other_long));  \
  return frame;                                                   \

#define INT_BOOL_OP(cxt, frame, self, op)              \
  Object *stack = get_stack(cxt, frame);               \
  TO_LONG(cxt, frame, self_long,  self);               \
  TO_LONG(cxt, frame, other_long, pop(cxt, stack));    \
  if ( self_long op other_long ) {                     \
    push(cxt, stack, get_true(cxt));                   \
  }                                                    \
  else {                                               \
    push(cxt, stack, get_false(cxt));                  \
  }                                                    \
  return frame;                                        \

Object* native_boxed_int_to_s(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  char buf[1024] = "";
  if ( is_boxed_int(cxt, self) ) {
    sprintf(buf, "%ld", boxed_int_to_fixnum(cxt, self));
  }
  push(cxt, stack, new_char_array(cxt, buf));
  return frame;
}

Object* native_println(Object *cxt, Object *frame, Object *self) {
  Object *arg = pop(cxt, get_stack(cxt, frame));
  if ( is_char_array(cxt, arg) ) {
    CharArrayBuffer *buf = get_char_array_buffer(arg);
    if ( buf != 0 ) {
      fprintf(stdout, "%s\n", buf->data);
    }
    else {
      return new_exception(cxt, frame, "Couldn't interpret char_array in println");
    }
  }
  else if ( is_boxed_int(cxt, arg) ) {
    BoxedIntBuffer *buf = get_boxed_int_buffer(arg);
    if ( buf != 0 ) {
      fprintf(stdout, "%ld\n", buf->value);
    }
    else {
      return new_exception(cxt, frame, "Couldn't interpret boxed int in println");
    }
  }
  else if ( arg == 0 ) {
      fprintf(stdout, "0x0\n");
  } 
  else {
    return new_exception(cxt, frame, "Couldn't interpret object in println");
  }
  push(cxt, get_stack(cxt, frame), get_undefined(cxt));
  return frame;
}

Object* native_object_equals(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *arg   = pop(cxt, stack);
  if ( self == arg ) {
    push(cxt, stack, get_true(cxt));
  }
  else {
    push(cxt, stack, get_false(cxt));
  }
  return frame;
}

Object* native_true(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  push(cxt, stack, get_true(cxt));
  return frame;
}

Object* native_false(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  push(cxt, stack, get_false(cxt));
  return frame;
}

Object* native_char_array_equals(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *arg   = pop(cxt, stack);
  if ( char_array_equals(self, arg) ) {
    push(cxt, stack, get_true(cxt));
  }
  else {
    push(cxt, stack, get_false(cxt));
  }
  return frame;
}

Object* native_char_array_append(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *arg   = pop(cxt, stack);
  if ( is_char_array(cxt, self) && is_char_array(cxt, arg)) {
    char_array_append(cxt, self, arg);
  }
  push(cxt, stack, self);
  return frame;
}

Object* native_char_array_concat(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *arg   = pop(cxt, stack);
  if ( is_char_array(cxt, self) && is_char_array(cxt, arg)) {
    push(cxt, stack, char_array_concat(cxt, self, arg));
  }
  else {
    push(cxt, stack, get_undefined(cxt));
  }
  return frame;
}

Object* native_boxed_int_equals(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *arg   = pop(cxt, stack);
  if ( boxed_int_equals(self, arg) ) {
    push(cxt, stack, get_true(cxt));
  }
  else {
    push(cxt, stack, get_false(cxt));
  }
  return frame;
}


Object* native_object_dispose(Object *cxt, Object *frame, Object *self) {
  object_dispose(cxt, self);
  return return_undefined(cxt, frame);
}

Object* native_object_get(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *arg = pop(cxt, stack);
  push(cxt, stack, get(cxt, self, arg));
  return frame;
}

Object* native_object_new(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  push(cxt, stack, new_object(cxt, self));
  return frame;
}

Object* native_raise(Object *cxt, Object *frame, Object *self) {
  return new_exception(cxt, frame, pop(cxt, get(cxt, frame, "stack")));
}

void dump(
  Object *cxt, Object *obj, Fixnum indent, Object *visited, 
  Fixnum max_indent
) {
  Fixnum i;

  char indent_char_array[indent+1];
  rjl_memset(indent_char_array, ' ', indent);
  indent_char_array[indent] = 0;

  if ( max_indent > 0 && indent > max_indent ) {
    return;
  };

  if ( obj == 0 ) {
    fprintf(stdout, "0\n");
  }
  else if ( is_char_array(cxt, obj) ) {
    fprintf(stdout, "\"%s\"\n", get_char_array_buffer(obj)->data);
  }
  else if ( is_func(cxt, obj) ) {
    fprintf(stdout, "FUNC: %p\n", get_func_buffer(obj)->func);
  }    
  else if ( is_boxed_int(cxt, obj) ) {
    fprintf(stdout, "BOXED_INT: %ld\n", get_boxed_int_buffer(obj)->value);
  }    
  else if ( is_true(cxt, get(cxt, visited, obj)) ) {
    fprintf(stdout, "^%p\n", obj);
  }
  else {
    set_true(cxt, visited, obj);
    if ( obj->occupied > 0 ) {
      fprintf(stdout, "{ %p \n", obj);
      for(i=0;i<obj->length;++i) {
        Object *key = obj->table[i].key;
        if ( key != 0 && key != DirtyKey ) {
          if ( is_char_array(cxt, key) ) {
            fprintf(stdout, "%s  %s: ", indent_char_array, 
                    get_char_array_buffer(key)->data);
            dump(cxt, obj->table[i].value, indent+2, visited, max_indent);
          }
          else {
            fprintf(stdout, "%s  %p: ", indent_char_array, key);
            dump(cxt, obj->table[i].value, indent+2, visited, max_indent);
          }
        }
      }
    }
    else {
      fprintf(stdout, "{ %p", obj);
    }

    if ( is_array(cxt, obj) ) {
      if ( array_length(cxt, obj) > 0 ) {
        if ( obj->occupied > 0 ) {
          fprintf(stdout, "%s  [\n", indent_char_array);
        }
        else {
          fprintf(stdout, " [\n");
        }
        for(i=0;i<array_length(cxt, obj);++i) {
          fprintf(stdout, "%s    %03d: ", indent_char_array, (int)i);
          dump(cxt, get_at(cxt, obj, i), indent+4, visited, max_indent);
        }
        fprintf(stdout, "%s  ]\n%s}\n", indent_char_array, indent_char_array);
      }
      else {
        if ( obj->occupied > 0 ) {
          fprintf(stdout, "%s  []\n%s}\n", indent_char_array, indent_char_array);
        }
        else {
          fprintf(stdout, " [] }\n");
        }
      }
    }
    else {
      if ( obj->occupied > 0 ) {
        fprintf(stdout, "%s}\n", indent_char_array);
      }
      else {
        fprintf(stdout, "}\n");
      }
    }
  }
} 

char *escape_string(char *dst, Fixnum len, char *src) {
  Fixnum i = 0;
  while(i+8 < len && *src) {
    if ( *src == 0 ) {
      dst[i++] = 0;
      return dst;
    }
    else if (
      ( *src >= '0' && *src <= '9' ) ||
      ( *src >= 'a' && *src <= 'z' ) ||
      ( *src >= 'A' && *src <= 'Z' ) ||
      ( *src == ' ' ) 
    ) {
      dst[i++] = *src;
    }
    else if ( *src < 100 ) {
      dst[i++] = '&';
      dst[i++] = '0' + (*src / 10);
      dst[i++] = '0' + (*src % 10);
      dst[i++] = ';';
    }
    else if ( *src < 100 ) {
      dst[i++] = '&';
      dst[i++] = '0' + (*src / 100);
      dst[i++] = '0' + ((*src % 100) / 10);
      dst[i++] = '0' + (*src % 10);
      dst[i++] = ';';
    }
    ++src;
  }
  dst[i++] = 0;
  return dst;
}

Fixnum is_dot_value(Object *cxt, Object *value) {
  return is_char_array(cxt, value) 
    || is_func(cxt, value) 
    || is_boxed_int(cxt, value) 
    || is_true(cxt, value) 
    || is_false(cxt, value)
    || is_undefined(cxt, value);
}

void dump_dot_value(FILE *fp, Object *cxt, Object *value) {
  char buf[4096]; // should be enough

  if ( is_char_array(cxt, value) ) {
    fprintf(fp, "'%s' ", 
            escape_string(buf, sizeof(buf), 
                          get_char_array_buffer(value)->data));
  }
  else if ( is_func(cxt, value) ) {
    fprintf(fp, "FUNC:%p ", get_func_buffer(value)->func);
  }
  else if ( is_boxed_int(cxt, value) ) {
    fprintf(fp, "%ld ", get_boxed_int_buffer(value)->value);
  }
  else if ( is_true(cxt, value) ) {
    fprintf(fp, "true ");
  }
  else if ( is_false(cxt, value) ) {
    fprintf(fp, "false ");
  }
  else if ( is_undefined(cxt, value) ) {
    fprintf(fp, "undefined ");
  }
}

void dump_dot_object(FILE *fp, Object *cxt, Object *obj) 
{
  Fixnum i;
  Object *key, *value;
  char buf[4024];

  if ( is_dot_value(cxt, obj) ) {
    return;
  }

  fprintf(fp, "\"obj%p\" [ ", obj);
  fprintf(fp, "shape = \"record\" label = \"%p ", obj);
  for(i=0;i<obj->length;++i) {
    key = obj->table[i].key;
    if ( key != 0 && key != DirtyKey ) {
      value = obj->table[i].value;
      fprintf(fp, "| <f%ld> ", i);
      if ( is_char_array(cxt, key) ) {
        fprintf(fp, "%s: ", 
                escape_string(buf, sizeof(buf), get_char_array_buffer(key)->data));
      }
      else {
        fprintf(fp, "%p: ", obj);
      }
      dump_dot_value(fp, cxt, value);
    }
  }
  if ( is_child_array(cxt, obj) ) {
    for(i=0;i<array_length(cxt, obj);++i) {
      value = get_at(cxt, obj, i);
      fprintf(fp, "| <g%ld> %ld: ", i, i);
      dump_dot_value(fp, cxt, value);
    }
  }
  fprintf(fp, "\"");
  fprintf(fp, "];\n");

  for(i=0;i<obj->length;++i) {
    Object *key = obj->table[i].key;
    if ( key != 0 && key != DirtyKey ) {
      Object *value = obj->table[i].value;
      if ( ! is_dot_value(cxt, value) ) {
        fprintf(fp, "\"obj%p\":f%ld -> \"obj%p\";\n", obj, i, value);
      }
    }
  }

  if ( is_child_array(cxt, obj) ) {
    for(i=0;i<array_length(cxt, obj);++i) {
      value = get_at(cxt, obj, i);
      if ( ! is_dot_value(cxt, value) ) {
        fprintf(fp, "\"obj%p\":g%ld -> \"obj%p\";\n", obj, i, value);
      }
    }
  }
} 

void dump_dot(Object *cxt) {
  FILE *fp = fopen("/tmp/objects.dot", "w");
  Object *objects = context_get(cxt, "objects");
  fprintf(fp, "digraph g {\ngraph [\nrankdir = \"LR\"\n];\n");
  for(Fixnum i=0;i<objects->length; ++i) {
    Object *key = objects->table[i].key;
    if ( key != 0 && key != DirtyKey && key != objects ) {
      dump_dot_object(fp, cxt, key);
    }
  }
  fprintf(fp, "}");
  fclose(fp);
}

void dump(Object *cxt, Object *obj) {
  dump(cxt, obj, 0, new_object(cxt), -1);
}

void dump(Object *cxt, Object *obj, Fixnum max_indent) {
  dump(cxt, obj, 0, new_object(cxt), max_indent);
}

Object* native_dump(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  dump(cxt, pop(cxt, stack));
  return return_undefined(cxt, frame);
}

Object* native_dump_to(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  dump(cxt, pop(cxt, stack), boxed_int_to_fixnum(cxt, pop(cxt, stack)));
  return return_undefined(cxt, frame);
}

Object *native_char_array_reserve(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object* new_size = pop(cxt, stack);
  if ( ! is_boxed_int(cxt, new_size) ) {
    return new_exception(cxt, frame, "Expected boxed int argument");
  }
  char_array_set_reserve(cxt, self, boxed_int_to_fixnum(cxt, new_size));
  return return_undefined(cxt, frame);
}

Object *native_char_array_new(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  push(cxt, stack, new_char_array(cxt, ""));
  return frame;
};

Object *native_char_array_shift(Object *cxt, Object *frame, Object *self) {
  Fixnum offset = boxed_int_to_fixnum(cxt, pop(cxt, get_stack(cxt, frame)));
  CharArrayBuffer *buf = get_char_array_buffer(self);
  if ( buf != 0 ) {
    if ( offset <= buf->length ) {
      rjl_memcpy(buf->data, buf->data + offset, buf->length - offset);
      char_array_truncate_buffer(cxt, self, buf->length - offset);
    }
    else {
      return new_exception(cxt, frame, "offset is larger than char_array length");
    }
  }
  else {
    return new_exception(cxt, frame, "expected self to be a char_array");
  }
  return return_undefined(cxt, frame);
};

Object *native_array_pop(Object *cxt, Object *frame, Object *self) {
  push(cxt, get_stack(cxt, frame), pop(cxt, self));
  return frame;
}

Object *native_array_push(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  push(cxt, self, pop(cxt, stack));
  return return_undefined(cxt, frame);
}

Object *native_array_at(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  TO_LONG(cxt, frame, other_long, pop(cxt, stack));
  push(cxt, stack, get_at(cxt, self, other_long));
  return frame;
}

Object *native_array_length(Object *cxt, Object *frame, Object *self) {
  push(cxt, get_stack(cxt, frame), new_boxed_int(cxt, array_length(cxt, self)));
  return frame;
}

Object *native_array_new(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *array = new_array(cxt);
  set(cxt, array, "parent", self);
  push(cxt, stack, array);
  return frame;
}

Object *native_boxed_int_leq(Object *cxt, Object *frame, Object *self) {
  INT_BOOL_OP(cxt, frame, self, <=);
}

Object *native_boxed_int_geq(Object *cxt, Object *frame, Object *self) {
  INT_BOOL_OP(cxt, frame, self, >=);
}

Object *native_boxed_int_gt(Object *cxt, Object *frame, Object *self) {
  INT_BOOL_OP(cxt, frame, self, >);
}

Object *native_boxed_int_le(Object *cxt, Object *frame, Object *self) {
  INT_BOOL_OP(cxt, frame, self, <);
}

Object *native_boxed_int_plus(Object *cxt, Object *frame, Object *self) {
  INT_INT_OP(cxt, frame, self, +);
}

Object *native_boxed_int_times(Object *cxt, Object *frame, Object *self) {
  INT_INT_OP(cxt, frame, self, *);
}

Object *native_boxed_int_minus(Object *cxt, Object *frame, Object *self) {
  INT_INT_OP(cxt, frame, self, -);
}

Object *native_block_call(Object *cxt, Object *frame, Object *self) {
  if ( is_slot_true(cxt, self, "is_block") ) {
    return new_frame(cxt, get_undefined(cxt), self, frame);
  }
  else {
    push(cxt, get(cxt, frame, "stack"), get_undefined(cxt));
    return frame;
  }
}

Object *native_block_invoke(Object *cxt, Object *frame, Object *block) {
  Object *stack = get(cxt, frame, "stack");
  Object *self = pop(cxt, stack);
  if ( is_slot_true(cxt, block, "is_block") ) {
    return new_frame(cxt, self, block, frame);
  }
  else {
    push(cxt, stack, get_undefined(cxt));
    return frame;
  }
}

Object *native_block_call1(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  if ( is_block(cxt, self) ) {
    return new_frame(cxt, get_undefined(cxt), self, frame);
  }
  else {
    pop(cxt, stack); // read the argument
    push(cxt, stack, get_undefined(cxt));
    return frame;
  }
}

Object *native_if_else(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *cond = pop(cxt, stack);
  Object *false_block = pop(cxt, stack);
  Object *true_block = pop(cxt, stack);
  Object *block = true_block;
  if ( ! is_true(cxt, cond) ) {
    block = false_block;
  }
  if ( is_block(cxt, block) ) {
    Object *ret_frame = new_frame(cxt, get_undefined(cxt), block, frame);
    return ret_frame;
  }
  else {
    return new_exception(cxt, frame, "Expected block arguments");
  }
};

Object *native_if(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  Object *cond = pop(cxt, stack);
  Object *true_block = pop(cxt, stack);
  if ( ! is_true(cxt, cond) ) {
    pop(cxt, stack); // read the return location from the stack
    return frame;
  }
  else {
    return new_frame(cxt, get_undefined(cxt), true_block, frame);
  }
};

Object* native_object_new1(Object *cxt) {
  Object *code = new_block(cxt);
  code_arg(cxt, code, "block");
  code_self_send(cxt, code, "self");
  code_send(cxt, code, "new");
  code_self_send(cxt, code, "result:");
  code_self_send(cxt, code, "result");
  code_self_send(cxt, code, "^block");
  code_send(cxt, code, "invoke:");
  code_self_send(cxt, code, "result");
  code_return(cxt, code);

  return code;
};

void init_native_sys(Object *cxt) {
  context_set(cxt, "println:", new_func(cxt, native_println));
  context_set(cxt, "dump:",    new_func(cxt, native_dump));
  context_set(cxt, "dump:to:", new_func(cxt, native_dump_to));
  context_set(cxt, "if:else:", new_func(cxt, native_if_else));
  context_set(cxt, "if:",      new_func(cxt, native_if));
  context_set(cxt, "raise:",   new_func(cxt, native_raise));

  Object *object = context_get(cxt, "Object");
  set(cxt, object, "get:",    new_func(cxt, native_object_get));
  set(cxt, object, "new",     new_func(cxt, native_object_new));
  set(cxt, object, "new:",    native_object_new1(cxt));
  set(cxt, object, "dispose", new_func(cxt, native_object_dispose));
  set(cxt, object, "==:",     new_func(cxt, native_object_equals));
  set(cxt, object, "is_true", new_func(cxt, native_true));

  Object *array = context_get(cxt, "Array");
  set(cxt, array, "pop",      new_func(cxt, native_array_pop));
  set(cxt, array, "push:",    new_func(cxt, native_array_push));
  set(cxt, array, "at:",      new_func(cxt, native_array_at));
  set(cxt, array, "length",   new_func(cxt, native_array_length));
  set(cxt, array, "new",      new_func(cxt, native_array_new));
  
  Object *block = context_get(cxt, "Block");
  set(cxt, block, "parent", array);
  set(cxt, block, "call",    new_func(cxt, native_block_call));
  set(cxt, block, "call:",   new_func(cxt, native_block_call1));
  set(cxt, block, "invoke:", new_func(cxt, native_block_invoke));

  Object *char_array = context_get(cxt, "CharArray");
  set(cxt, char_array, "new",      new_func(cxt, native_char_array_new));
  set(cxt, char_array, "reserve:", new_func(cxt, native_char_array_reserve));
  set(cxt, char_array, "shift:",   new_func(cxt, native_char_array_shift));
  set(cxt, char_array, "==:",      new_func(cxt, native_char_array_equals));
  set(cxt, char_array, "<<:",      new_func(cxt, native_char_array_append));
  set(cxt, char_array, "+:",       new_func(cxt, native_char_array_concat));

  Object *boxed_int = context_get(cxt, "BoxedInt");
  set(cxt, boxed_int, "<=:",  new_func(cxt, native_boxed_int_leq));
  set(cxt, boxed_int, ">=:",  new_func(cxt, native_boxed_int_geq));
  set(cxt, boxed_int, ">:",   new_func(cxt, native_boxed_int_gt));
  set(cxt, boxed_int, "<:",   new_func(cxt, native_boxed_int_le));
  set(cxt, boxed_int, "+:",   new_func(cxt, native_boxed_int_plus));
  set(cxt, boxed_int, "-:",   new_func(cxt, native_boxed_int_minus));
  set(cxt, boxed_int, "*:",   new_func(cxt, native_boxed_int_times));
  set(cxt, boxed_int, "==:",  new_func(cxt, native_boxed_int_equals));
  set(cxt, boxed_int, "to_s", new_func(cxt, native_boxed_int_to_s));

  Object *native_false_func = new_func(cxt, native_false);
  Object *undefined_object = get_undefined(cxt);
  set(cxt, undefined_object, "is_true", native_false_func);

  Object *false_object = get_false(cxt);
  set(cxt, false_object, "is_true", native_false_func);
};
