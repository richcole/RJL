#include "char_array.h"
#include "context.h"
#include "mem.h"
#include "sym.h"
#include "type_tags.h"

CharArrayBuffer *new_char_array_buffer(Object *cxt, Fixnum len) {
  CharArrayBuffer *buf = (CharArrayBuffer *)context_alloc_buffer(cxt, sizeof(CharArrayBuffer)+len+1);
  buf->type      = CharArrayTypeTag;
  buf->length    = 0;
  buf->reserved  = len;
  return buf;
};

CharArrayBuffer *new_char_array_buffer(Object *cxt, char const* s, Fixnum len) {
  CharArrayBuffer *buf = new_char_array_buffer(cxt, len);
  buf->data[len] = 0;
  rjl_memcpy(buf->data, s, len);
  buf->length = len;
  return buf;
}

CharArrayBuffer *new_char_array_buffer(Object *cxt, char const* s) {
  return new_char_array_buffer(cxt, s, rjl_strlen(s));
}

Object *char_array(Object *cxt) {
  return context_get(cxt, "CharArray");
}

Object *new_char_array(Object *cxt, char const* s) {
  Object *obj = new_object(cxt, char_array(cxt));
  obj->buffer = (Buffer *) new_char_array_buffer(cxt, s);
  return obj;
}

Object *new_char_array_no_register(Object *cxt, char const* s) {
  Object *obj = new_object_no_register();
  obj->buffer = (Buffer *) new_char_array_buffer(cxt, s);
  return obj;
}

Object *new_char_array(Object *cxt, Fixnum reserved) {
  Object *obj = new_object(cxt, char_array(cxt));
  obj->buffer = (Buffer *) new_char_array_buffer(cxt, reserved);
  return obj;
}

def_get_buffer(CharArray, char_array, CharArrayTypeTag);
def_set_buffer(CharArray, char_array, CharArrayTypeTag);

Fixnum is_char_array(Object *cxt, Object *obj) {
  return get_char_array_buffer(obj) != 0; 
}

Fixnum char_array_equals(Object *s1, Object *s2) {
  CharArrayBuffer *sb1 = get_char_array_buffer(s1);
  CharArrayBuffer *sb2 = get_char_array_buffer(s2);
	
	if ( sb1 == 0 || sb2 == 0 ) {
		return 0;
	}
	if ( sb1->length != sb2->length ) {
		return 0;
	}
	for(Fixnum i=0;i<sb1->length;++i) {
		if ( sb1->data[i] != sb2->data[i] ) {
			return 0;
		}
	}
	return 1;
}

Fixnum char_array_equals(Object *s1, char const* s2) {
  CharArrayBuffer *sb1 = get_char_array_buffer(s1);

  if ( sb1 == 0 || s2 == 0 ) {
    return 0;
  }
  
  Fixnum i;
  for(i=0; i<sb1->length; ++i) {
    if ( (s2[i] == 0) || (sb1->data[i] != s2[i]) ) {
      return 0;
    }
  }
  if ( s2[i] == 0 ) { 
    return 1;
  }
  else {
    return 0;
  }
}

void char_array_truncate_buffer(Object *cxt, Object *str, Fixnum len) {
  CharArrayBuffer *buf = get_char_array_buffer(str);
  if ( buf != 0 && len <= buf->reserved ) {
    buf->length = len;
    buf->data[buf->length] = 0;
  }
}

void char_array_set_reserve(Object *cxt, Object *str, Fixnum new_size) {
  CharArrayBuffer *cb = get_char_array_buffer(str);
  CharArrayBuffer *nb = new_char_array_buffer(cxt, new_size);
  Fixnum len = new_size;

  if ( cb->length < len ) {
    len = cb->length;
  }

  rjl_memcpy(nb->data, cb->data, len);
  nb->length = len;
  nb->data[nb->length] = 0;

  set_char_array_buffer(str, nb);
  context_free_buffer(cxt, cb);
}

Fixnum char_array_length(Object *cxt, Object *str) {
  CharArrayBuffer *buf = get_char_array_buffer(str);
  if ( buf != 0 ) {
    return buf->length;
  }
  else {
    return 0;
  }
}

char char_array_get_at(Object *cxt, Object *str, Fixnum index) {
  CharArrayBuffer *buf = get_char_array_buffer(str);
  if ( buf != 0 && index < buf->length ) {
    return buf->data[index];
  }
  else {
    return 0;
  }
};

Fixnum char_array_reserve(Object *cxt, Object *str) {
  CharArrayBuffer *buf = get_char_array_buffer(str);
  if ( buf != 0 ) {
    return buf->reserved;
  }
  else {
    return 0;
  }
}

Object *char_array_subchar_array(Object *cxt, Object *char_array, Fixnum start, Fixnum end) {
  CharArrayBuffer *buf = get_char_array_buffer(char_array);
  if ( buf != 0 && start < buf->length && end <= buf->length ) {
    Object *ret = new_char_array(cxt, end - start);
    CharArrayBuffer *ret_buf = get_char_array_buffer(ret);
    rjl_memcpy(ret_buf->data, buf->data + start, end - start);
    ret_buf->length = end - start;
    return ret;
  }
  return 0;
}

void char_array_append(Object *cxt, Object *str, Object *arg) {
  CharArrayBuffer *str_buf = get_char_array_buffer(str);
  CharArrayBuffer *arg_buf = get_char_array_buffer(arg);
  if ( arg_buf == 0 || str_buf == 0 ) {
    return;
  }
  Fixnum new_length = str_buf->length + arg_buf->length;
  if ( new_length > str_buf->reserved ) {
    char_array_set_reserve(cxt, str, new_length);
    str_buf = get_char_array_buffer(str);
  }
  rjl_memcpy(str_buf->data + str_buf->length, arg_buf->data, arg_buf->length);
  str_buf->length = new_length;
  str_buf->data[new_length] = 0;
}

Object* char_array_concat(Object *cxt, Object *str, Object *arg) {
  Fixnum len = char_array_length(cxt, str) + char_array_length(cxt, arg);
  Object *ret = new_char_array(cxt, len);
  CharArrayBuffer *ret_buf = get_char_array_buffer(ret);
  CharArrayBuffer *str_buf = get_char_array_buffer(str);
  CharArrayBuffer *arg_buf = get_char_array_buffer(arg);
  if ( ret_buf == 0 || arg_buf == 0 || str_buf == 0 ) {
    return 0;
  }
  rjl_memcpy(ret_buf->data, str_buf->data, str_buf->length);
  rjl_memcpy(ret_buf->data + str_buf->length, arg_buf->data, arg_buf->length);
  ret_buf->data[len] = 0;
  ret_buf->length = len;
  return ret;
};







