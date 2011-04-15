
Object *String = new_object();
Object *StringObject = new_object();

struct StringBuffer {
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

StringBuffer *new_string_buffer(Fixnum len) {
  StringBuffer *buf = (StringBuffer *)mem_alloc(sizeof(StringBuffer)+len+1);
  buf->type      = String;
  buf->length    = 0;
  buf->reserved  = len;
  return buf;
};

StringBuffer *new_string_buffer(char const* s, Fixnum len) {
  StringBuffer *buf = new_string_buffer(len);
  buf->data[len] = 0;
  rjl_memcpy(buf->data, s, len);
  buf->length = len;
  return buf;
};

StringBuffer *new_string_buffer(char const* s) {
  return new_string_buffer(s, rjl_strlen(s));
}

Object *new_string(char const* s) {
  Object *obj = new_object();
  obj->buffer = (Buffer *) new_string_buffer(s);
  set(obj, Parent, StringObject);
  return obj;
}

Object *new_string(Fixnum reserved) {
  Object *obj = new_object();
  obj->buffer = (Buffer *) new_string_buffer(reserved);
  set(obj, Parent, StringObject);
  return obj;
}

def_get_buffer(String, string);
def_set_buffer(String, string);

Fixnum is_string(Object *obj) {
  if ( obj != 0 && is_object(obj) && obj->buffer && obj->buffer->type == String ) {
    return 1;
  }
  return 0;
}

Fixnum string_equals(Object *s1, Object *s2) {
	StringBuffer *sb1 = get_string_buffer(s1);
	StringBuffer *sb2 = get_string_buffer(s2);
	
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

Fixnum string_equals(Object *s1, char const* s2) {
	StringBuffer *sb1 = get_string_buffer(s1);

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

void string_truncate_buffer(Object *str, Fixnum len) {
  StringBuffer *buf = get_string_buffer(str);
  if ( buf != 0 && len <= buf->reserved ) {
    buf->length = len;
    buf->data[buf->length] = 0;
  }
}

void string_set_reserve(Object *str, Fixnum new_size) {
  StringBuffer *cb = get_string_buffer(str);
  StringBuffer *nb = new_string_buffer(new_size);
  Fixnum len = new_size;

  if ( cb->length < len ) {
    len = cb->length;
  }

  rjl_memcpy(nb->data, cb->data, len);
  nb->length = len;
  nb->data[nb->length] = 0;

  set_string_buffer(str, nb);
  mem_free(cb);
}

Fixnum string_length(Object *str) {
  StringBuffer *buf = get_string_buffer(str);
  if ( buf != 0 ) {
    return buf->length;
  }
  else {
    return 0;
  }
}

char string_get_at(Object *str, Fixnum index) {
  StringBuffer *buf = get_string_buffer(str);
  if ( buf != 0 && index < buf->length ) {
    return buf->data[index];
  }
  else {
    return 0;
  }
};

Fixnum string_reserve(Object *str) {
  StringBuffer *buf = get_string_buffer(str);
  if ( buf != 0 ) {
    return buf->reserved;
  }
  else {
    return 0;
  }
}

Object *string_substring(Object *string, Fixnum start, Fixnum end) {
  StringBuffer *buf = get_string_buffer(string);
  if ( buf != 0 && start < buf->length && end <= buf->length ) {
    Object *ret = new_string(end - start);
    StringBuffer *ret_buf = get_string_buffer(ret);
    rjl_memcpy(ret_buf->data, buf->data + start, end - start);
    ret_buf->length = end - start;
    return ret;
  }
  return 0;
}

void string_append(Object *str, Object *arg) {
  StringBuffer *str_buf = get_string_buffer(str);
  StringBuffer *arg_buf = get_string_buffer(arg);
  if ( arg_buf == 0 || str_buf == 0 ) {
    return;
  }
  Fixnum new_length = str_buf->length + arg_buf->length;
  if ( new_length > str_buf->reserved ) {
    string_set_reserve(str, new_length);
    str_buf = get_string_buffer(str);
  }
  rjl_memcpy(str_buf->data + str_buf->length, arg_buf->data, arg_buf->length);
  str_buf->length = new_length;
  str_buf->data[new_length] = 0;
}

Object* string_concat(Object *str, Object *arg) {
  Fixnum len = string_length(str) + string_length(arg);
  Object *ret = new_string(len);
  StringBuffer *ret_buf = get_string_buffer(ret);
  StringBuffer *str_buf = get_string_buffer(str);
  StringBuffer *arg_buf = get_string_buffer(arg);
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

void init_string_symbols() {
	add_sym(String, "String");
}




