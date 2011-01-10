
#include <stdio.h>

Object* native_print(Object *frame) {
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

void dump(Object *obj, Fixnum indent) {
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
  else if ( is_object(obj) ) {
    if ( obj->occupied > 0 ) {
      fprintf(stdout, "{\n");
      for(i=0;i<obj->length;++i) {
        Object *key = obj->table[i].key;
        if ( key != 0 && key != Dirty ) {
          if ( is_string(key) ) {
            fprintf(stdout, "%s  %s: ", indent_string, get_string_buffer(key)->data);
            dump(obj->table[i].value, indent+2);
          }
          else {
            fprintf(stdout, "%s  %p: ", indent_string, key);
            dump(obj->table[i].value, indent+2);
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
          fprintf(stdout, "%s    ", indent_string);
          dump(get_at(obj, i), indent+4);
        }
        fprintf(stdout, "%s] }\n", indent_string);
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
  dump(obj, 0);
}

Object *native_string_reserve(Object *frame) {
  Object* new_size = pop(get(frame, Stack));
  if ( ! is_fixnum(new_size) ) {
    return new_exception(frame, "Expected fixnum argument");
  }
  string_set_reserve(get(frame, Self), fixnum(new_size));
  push(get(frame, Stack), Nil);
  return frame;
}

Object *native_string_shift(Object *frame) {
  Fixnum offset = fixnum(pop(get(frame, Stack)));
  Object *self = get(frame, Self);
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

void init_native_sys(Object *sys) {
  set(sys, sym("println:"), new_func(native_print));

  Object *string_object = new_object();
  set(sys, String, string_object);
  set(string_object, sym("reserve:"), new_func(native_string_reserve));
  set(string_object, sym("shift:"), new_func(native_string_shift));
};
