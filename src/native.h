
#include <stdio.h>

Object *to_long(Object *frame, Object *obj, long *num) {
  if ( is_fixnum(obj) ) {
    *num = fixnum(obj);
    return 0;
  }
  else {
    BoxedIntBuffer *obj_buf  = get_boxed_int_buffer(obj);
    if ( obj != 0 ) {
      *num = obj_buf->value;
      return 0;
    }
    else {
      return new_exception(frame, "expected fixnum or boxed_int");
    }
  }
}

#define TO_LONG(frame, var, value)                    \
  long var;                                           \
  {                                                   \
    Object *ex = to_long(frame, value, &var);         \
    if ( ex != 0 ) {                                  \
      return ex;                                      \
    };                                                \
  };                                                  \

#define INT_INT_OP(frame, self, op)                     \
  Object *stack = get(frame, Stack);                    \
  TO_LONG(frame, self_long,  self);                     \
  TO_LONG(frame, other_long, pop(stack));               \
  push(stack, new_boxed_int(self_long op other_long));  \
  return frame;                                         \

#define INT_BOOL_OP(frame, self, op)                  \
  Object *stack = get(frame, Stack);                  \
  TO_LONG(frame, self_long,  self);                   \
  TO_LONG(frame, other_long, pop(stack));             \
  if ( self_long op other_long ) {                    \
    push(stack, True);                                \
  }                                                   \
  else {                                              \
    push(stack, False);                               \
  }                                                   \
  return frame;                                       \

Object* native_println(Object *frame, Object *self) {
  Object *arg = pop(get(frame, Stack));
  if ( is_string(arg) ) {
    StringBuffer *buf = get_string_buffer(arg);
    if ( buf != 0 ) {
      fprintf(stdout, "%s\n", buf->data);
    }
    else {
      return new_exception(frame, "Couldn't interpret string in println");
    }
  }
  else if ( is_fixnum(arg) ) {
    fprintf(stdout, "%ld\n", fixnum(arg));
  }
  else if ( is_boxed_int(arg) ) {
    BoxedIntBuffer *buf = get_boxed_int_buffer(arg);
    if ( buf != 0 ) {
      fprintf(stdout, "%ld\n", buf->value);
    }
    else {
      return new_exception(frame, "Couldn't interpret boxed int in println");
    }
  }
  else if ( arg == 0 ) {
      fprintf(stdout, "0x0\n");
  } 
  else {
    return new_exception(frame, "Couldn't interpret object in println");
  }
  push(get(frame, Stack), Nil);
  return frame;
}

Object* native_object_get(Object *frame, Object *self) {
  Object *stack = get(frame, Stack);
  Object *arg = pop(stack);
  push(stack, get(self, arg));
  return frame;
}

Object* native_object_new(Object *frame, Object *self) {
  Object *stack = get(frame, Stack);
  push(stack, new_object(self));
  return frame;
}

void dump(Object *obj, Fixnum indent, Object *visited, Fixnum max_indent) {
  Fixnum i;

  char indent_string[indent+1];
  memset(indent_string, ' ', indent);
  indent_string[indent] = 0;

  if ( max_indent > 0 && indent > max_indent ) {
    return;
  };

  if ( obj == 0 ) {
    fprintf(stdout, "0\n");
  }
  else if ( is_fixnum(obj) ) {
    fprintf(stdout, "%ld\n", fixnum(obj));
  }
  else if ( is_string(obj) ) {
    fprintf(stdout, "\"%s\"\n", get_string_buffer(obj)->data);
  }
  else if ( is_func(obj) ) {
    fprintf(stdout, "FUNC: %p\n", get_func_buffer(obj)->func);
  }    
  else if ( is_boxed_int(obj) ) {
    fprintf(stdout, "BOXED_INT: %ld\n", get_boxed_int_buffer(obj)->value);
  }    
  else if ( get(visited, obj) == True ) {
    fprintf(stdout, "^%p\n", obj);
  }
  else if ( is_object(obj) ) {
    set(visited, obj, True);
    if ( obj->occupied > 0 ) {
      fprintf(stdout, "{ %p \n", obj);
      for(i=0;i<obj->length;++i) {
        Object *key = obj->table[i].key;
        if ( key != 0 && key != Dirty ) {
          if ( is_string(key) ) {
            fprintf(stdout, "%s  %s: ", indent_string, get_string_buffer(key)->data);
            dump(obj->table[i].value, indent+2, visited, max_indent);
          }
          else {
            fprintf(stdout, "%s  %p: ", indent_string, key);
            dump(obj->table[i].value, indent+2, visited, max_indent);
          }
        }
      }
    }
    else {
      fprintf(stdout, "{ %p", obj);
    }

    if ( is_array(obj) ) {
      if ( array_length(obj) > 0 ) {
        if ( obj->occupied > 0 ) {
          fprintf(stdout, "%s  [\n", indent_string);
        }
        else {
          fprintf(stdout, " [\n");
        }
        for(i=0;i<array_length(obj);++i) {
          fprintf(stdout, "%s    %03d: ", indent_string, (int)i);
          dump(get_at(obj, i), indent+4, visited, max_indent);
        }
        fprintf(stdout, "%s  ]\n%s}\n", indent_string, indent_string);
      }
      else {
        if ( obj->occupied > 0 ) {
          fprintf(stdout, "%s  []\n%s}\n", indent_string, indent_string);
        }
        else {
          fprintf(stdout, " [] }\n");
        }
      }
    }
    else {
      if ( obj->occupied > 0 ) {
        fprintf(stdout, "%s}\n", indent_string);
      }
      else {
        fprintf(stdout, "}\n");
      }
    }
  }
} 

void dump(Object *obj) {
  dump(obj, 0, new_object(), -1);
}

void dump(Object *obj, Fixnum max_indent) {
  dump(obj, 0, new_object(), max_indent);
}

Object* native_dump(Object *frame, Object *self) {
  Object *stack = get(frame, Stack);
  dump(pop(stack));
  push(stack, Nil);
  return frame;
}

Object *native_string_reserve(Object *frame, Object *self) {
  Object *stack = get(frame, Stack);
  Object* new_size = pop(stack);
  if ( ! is_fixnum(new_size) ) {
    return new_exception(frame, "Expected fixnum argument");
  }
  string_set_reserve(self, fixnum(new_size));
  push(stack, Nil);
  return frame;
}

Object *native_string_shift(Object *frame, Object *self) {
  Fixnum offset = fixnum(pop(get(frame, Stack)));
  StringBuffer *buf = get_string_buffer(self);
  if ( buf != 0 ) {
    if ( offset < buf->length ) {
      rjl_memcpy(buf->data, buf->data + offset, buf->length - offset);
      string_truncate_buffer(self, buf->length - offset);
    }
    else {
      return new_exception(frame, "offset is larger than string length");
    }
  }
  else {
    return new_exception(frame, "expected self to be a string");
  }
  return frame;
};

Object *native_array_pop(Object *frame, Object *self) {
  push(get(frame, Stack), pop(self));
  return frame;
}

Object *native_array_push(Object *frame, Object *self) {
  Object *stack = get(frame, Stack);
  push(self, pop(stack));
  push(stack, Nil);
  return frame;
}

Object *native_array_at(Object *frame, Object *self) {
  Object *stack = get(frame, Stack);
  TO_LONG(frame, other_long, pop(stack));
  push(stack, get_at(self, other_long));
  return frame;
}

Object *native_array_length(Object *frame, Object *self) {
  push(get(frame, Stack), object(array_length(self)));
  return frame;
}

Object *native_array_new(Object *frame, Object *self) {
  Object *stack = get(frame, Stack);
  Object *array = new_object(self);
  array->buffer = (Buffer *) new_array_buffer(10);
  push(stack, array);
  return frame;
}

Object *native_boxed_int_leq(Object *frame, Object *self) {
  INT_BOOL_OP(frame, self, <=);
}

Object *native_boxed_int_geq(Object *frame, Object *self) {
  INT_BOOL_OP(frame, self, >=);
}

Object *native_boxed_int_gt(Object *frame, Object *self) {
  INT_BOOL_OP(frame, self, >);
}

Object *native_boxed_int_le(Object *frame, Object *self) {
  INT_BOOL_OP(frame, self, <);
}

Object *native_boxed_int_plus(Object *frame, Object *self) {
  INT_INT_OP(frame, self, +);
}

Object *native_boxed_int_times(Object *frame, Object *self) {
  INT_INT_OP(frame, self, *);
}

Object *native_boxed_int_minus(Object *frame, Object *self) {
  INT_INT_OP(frame, self, -);
}

Object *native_block_call(Object *frame, Object *self) {
  if ( get(self, "is_block") == True ) {
    return new_frame(Undefined, self, frame);
  }
  else {
    push(get(frame, "stack"), Undefined);
    return frame;
  }
}

Object *native_block_invoke(Object *frame, Object *block) {
  Object *stack = get(frame, "stack");
  Object *self = pop(stack);
  if ( get(block, "is_block") == True ) {
    return new_frame(self, block, frame);
  }
  else {
    push(stack, Undefined);
    return frame;
  }
}

Object *native_block_call1(Object *frame, Object *self) {
  Object *stack = get(frame, Stack);
  if ( get(self, "is_block") == True ) {
    return new_frame(Undefined, self, frame);
  }
  else {
    pop(stack); // read the argument
    push(stack, Undefined);
    return frame;
  }
}

Object *native_if_else(Object *frame, Object *self) {
  Object *stack = get(frame, Stack);
  Object *cond = pop(stack);
  Object *false_block = pop(stack);
  Object *true_block = pop(stack);
  Object *block = true_block;
  if ( cond != True ) {
    block = false_block;
  }
  if ( get(block, "is_block") == True ) {
    Object *ret_frame = new_frame(Undefined, block, frame);
    return ret_frame;
  }
  else {
    return frame;
  }
};

Object* native_object_new1() {
  Object *code = new_block();
  code_arg(code, "block");
  code_self_send(code, "self");
  code_send(code, "new");
  code_self_send(code, "result:");
  code_self_send(code, "result");
  code_self_send(code, "^block");
  code_send(code, "invoke:");
  code_self_send(code, "result");
  code_return(code);

  return code;
};

void init_native_sys(Object *sys) {
  set(sys, sym("println:"), new_func(native_println));
  set(sys, sym("dump:"), new_func(native_dump));
  set(sys, sym("if:else:"), new_func(native_if_else));

  set(sys, sym("Object"), ObjectObject);
  set(ObjectObject, sym("get:"), new_func(native_object_get));
  set(ObjectObject, sym("new"), new_func(native_object_new));
  set(ObjectObject, sym("new:"), native_object_new1());

  set(sys, sym("Block"), BlockObject);
  set(BlockObject, sym("call"), new_func(native_block_call));
  set(BlockObject, sym("call:"), new_func(native_block_call1));
  set(BlockObject, sym("invoke:"), new_func(native_block_invoke));

  set(sys, String, StringObject);
  set(StringObject, sym("reserve:"), new_func(native_string_reserve));
  set(StringObject, sym("shift:"), new_func(native_string_shift));

  set(sys, Array, ArrayObject);
  set(ArrayObject, sym("pop"), new_func(native_array_pop));
  set(ArrayObject, sym("push:"), new_func(native_array_push));
  set(ArrayObject, sym("at:"), new_func(native_array_at));
  set(ArrayObject, sym("length"), new_func(native_array_length));
  set(ArrayObject, sym("new"), new_func(native_array_new));
  
  set(sys, "BoxedInt", BoxedIntObject);
  set(BoxedIntObject, "<=:", new_func(native_boxed_int_leq));
  set(BoxedIntObject, ">=:", new_func(native_boxed_int_geq));
  set(BoxedIntObject, ">:",  new_func(native_boxed_int_gt));
  set(BoxedIntObject, "<:",  new_func(native_boxed_int_le));
  set(BoxedIntObject, "+:",  new_func(native_boxed_int_plus));
  set(BoxedIntObject, "-:",  new_func(native_boxed_int_minus));
  set(BoxedIntObject, "*:",  new_func(native_boxed_int_times));
};
