
Object *CharArray = new_object();
Object *CharArrayObject = new_object();

struct CharArrayBuffer {
  Object *type;
  Fixnum reserved;
  Fixnum length;
  char   data[0];
};

Fixnum rjl_strlen(char const* s) {
  Fixnum i = 0;
  while(s[i] != 0) ++i;
  return i;
}

void rjl_memcpy(void *dst, void const* src, int len) {
  char *p = (char *)dst;
  char *q = (char *)src;
  while(len-- > 0) {
    *p++ = *q++;
  }
}

CharArrayBuffer *new_char_array_buffer(Fixnum len) {
  CharArrayBuffer *buf = (CharArrayBuffer *)mem_alloc(sizeof(CharArrayBuffer)+len+1);
  buf->type      = CharArray;
  buf->length    = 0;
  buf->reserved  = len;
  return buf;
};

CharArrayBuffer *new_char_array_buffer(char const* s, Fixnum len) {
  CharArrayBuffer *buf = new_char_array_buffer(len);
  buf->data[len] = 0;
  rjl_memcpy(buf->data, s, len);
  buf->length = len;
  return buf;
};

CharArrayBuffer *new_char_array_buffer(char const* s) {
  return new_char_array_buffer(s, rjl_strlen(s));
}

Object *new_char_array(char const* s) {
  Object *obj = new_object();
  obj->buffer = (Buffer *) new_char_array_buffer(s);
  set(obj, Parent, CharArrayObject);
  return obj;
}

Object *new_char_array(Fixnum reserved) {
  Object *obj = new_object();
  obj->buffer = (Buffer *) new_char_array_buffer(reserved);
  set(obj, Parent, CharArrayObject);
  return obj;
}

def_get_buffer(CharArray, char_array);
def_set_buffer(CharArray, char_array);

Fixnum is_char_array(Object *obj) {
  if ( obj != 0 && is_object(obj) && obj->buffer && obj->buffer->type == CharArray ) {
    return 1;
  }
  return 0;
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

void char_array_truncate_buffer(Object *str, Fixnum len) {
  CharArrayBuffer *buf = get_char_array_buffer(str);
  if ( buf != 0 && len <= buf->reserved ) {
    buf->length = len;
    buf->data[buf->length] = 0;
  }
}

void char_array_set_reserve(Object *str, Fixnum new_size) {
  CharArrayBuffer *cb = get_char_array_buffer(str);
  CharArrayBuffer *nb = new_char_array_buffer(new_size);
  Fixnum len = new_size;

  if ( cb->length < len ) {
    len = cb->length;
  }

  rjl_memcpy(nb->data, cb->data, len);
  nb->length = len;
  nb->data[nb->length] = 0;

  set_char_array_buffer(str, nb);
  mem_free(cb);
}

Fixnum char_array_length(Object *str) {
  CharArrayBuffer *buf = get_char_array_buffer(str);
  if ( buf != 0 ) {
    return buf->length;
  }
  else {
    return 0;
  }
}

char char_array_get_at(Object *str, Fixnum index) {
  CharArrayBuffer *buf = get_char_array_buffer(str);
  if ( buf != 0 && index < buf->length ) {
    return buf->data[index];
  }
  else {
    return 0;
  }
};

Fixnum char_array_reserve(Object *str) {
  CharArrayBuffer *buf = get_char_array_buffer(str);
  if ( buf != 0 ) {
    return buf->reserved;
  }
  else {
    return 0;
  }
}

Object *char_array_subchar_array(Object *char_array, Fixnum start, Fixnum end) {
  CharArrayBuffer *buf = get_char_array_buffer(char_array);
  if ( buf != 0 && start < buf->length && end <= buf->length ) {
    Object *ret = new_char_array(end - start);
    CharArrayBuffer *ret_buf = get_char_array_buffer(ret);
    rjl_memcpy(ret_buf->data, buf->data + start, end - start);
    ret_buf->length = end - start;
    return ret;
  }
  return 0;
}

void char_array_append(Object *str, Object *arg) {
  CharArrayBuffer *str_buf = get_char_array_buffer(str);
  CharArrayBuffer *arg_buf = get_char_array_buffer(arg);
  if ( arg_buf == 0 || str_buf == 0 ) {
    return;
  }
  Fixnum new_length = str_buf->length + arg_buf->length;
  if ( new_length > str_buf->reserved ) {
    char_array_set_reserve(str, new_length);
    str_buf = get_char_array_buffer(str);
  }
  rjl_memcpy(str_buf->data + str_buf->length, arg_buf->data, arg_buf->length);
  str_buf->length = new_length;
  str_buf->data[new_length] = 0;
}

Object* char_array_concat(Object *str, Object *arg) {
  Fixnum len = char_array_length(str) + char_array_length(arg);
  Object *ret = new_char_array(len);
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


// forward decl
void add_sym(Object *obj, char const* str);

void init_char_array_symbols() {
	add_sym(CharArray, "CharArray");
}




