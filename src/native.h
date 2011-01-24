
#include <stdio.h>

Object* native_print(Object *frame, Object *self) {
  Object *arg = pop(get(frame, Stack));
  StringBuffer *buf = get_string_buffer(arg);
  if ( buf != 0 ) {
    fprintf(stdout, "%s\n", buf->data);
  }
  else {
    return new_exception(frame, "Expected a string in native_print");
  }
  push(get(frame, Stack), Nil);
  return frame;
}

void dump(Object *obj, Fixnum indent, Object *visited) {
  Fixnum i;

  char indent_string[indent+1];
  memset(indent_string, ' ', indent);
  indent_string[indent] = 0;

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
  else if ( get(visited, obj) == True ) {
    fprintf(stdout, "^%p\n", obj);
  }
  else if ( is_object(obj) ) {
    set(visited, obj, True);
    if ( obj->occupied > 0 ) {
      fprintf(stdout, "{\n");
      for(i=0;i<obj->length;++i) {
        Object *key = obj->table[i].key;
        if ( key != 0 && key != Dirty ) {
          if ( is_string(key) ) {
            fprintf(stdout, "%s  %s: ", indent_string, get_string_buffer(key)->data);
            dump(obj->table[i].value, indent+2, visited);
          }
          else {
            fprintf(stdout, "%s  %p: ", indent_string, key);
            dump(obj->table[i].value, indent+2, visited);
          }
        }
      }
    }
    else {
      fprintf(stdout, "{");
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
          dump(get_at(obj, i), indent+4, visited);
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
  dump(obj, 0, new_object());
}

Object *native_string_reserve(Object *frame, Object *self) {
  Object* new_size = pop(get(frame, Stack));
  if ( ! is_fixnum(new_size) ) {
    return new_exception(frame, "Expected fixnum argument");
  }
  string_set_reserve(self, fixnum(new_size));
  push(get(frame, Stack), Nil);
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
  push(self, pop(get(frame, Stack)));
  push(get(frame, Stack), Nil);
  return frame;
}

Object *native_array_length(Object *frame, Object *self) {
  push(get(frame, Stack), object(array_length(self)));
  return frame;
}


void init_native_sys(Object *sys) {
  set(sys, sym("println:"), new_func(native_print));

  set(sys, String, StringObject);
  set(StringObject, sym("reserve:"), new_func(native_string_reserve));
  set(StringObject, sym("shift:"), new_func(native_string_shift));

  set(sys, Array, ArrayObject);
  set(ArrayObject, sym("pop"), new_func(native_array_pop));
  set(ArrayObject, sym("push:"), new_func(native_array_push));
  set(ArrayObject, sym("length"), new_func(native_array_length));
  
  
};
