
Object *Parent = new_object();
Object *True = new_object();
Object *False = new_object();
Object *String = new_object();

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
  buf->length    = len+1;
  buf->reserved  = buf->length;
  return buf;
};

StringBuffer *new_string_buffer(char const* s, Fixnum len) {
  StringBuffer *buf = new_string_buffer(len);
  buf->data[len] = 0;
  rjl_memcpy(buf->data, s, len);
  return buf;
};

StringBuffer *new_string_buffer(char const* s) {
  return new_string_buffer(s, rjl_strlen(s));
}

Object *new_string(char const* s) {
  Object *obj = new_object();
  obj->buffer = (Buffer *) new_string_buffer(s);
  return obj;
}

def_get_buffer(String, string);

Fixnum is_string(Object *obj) {
  if ( obj && obj->buffer && obj->buffer->type == String ) {
    return 1;
  }
  return 0;
}

Object* string_equals(Object *s1, Object *s2) {
	StringBuffer *sb1 = get_string_buffer(s1);
	StringBuffer *sb2 = get_string_buffer(s2);
	
	if ( sb1 == 0 || sb2 == 0 ) {
		return False;
	}
	if ( sb1->length != sb2->length ) {
		return False;
	}
	for(Fixnum i=0;i<sb1->length;++i) {
		if ( sb1->data[i] != sb2->data[i] ) {
			return False;
		}
	}
	return True;
}

Object* string_equals(Object *s1, char const* s2) {
	StringBuffer *sb1 = get_string_buffer(s1);

	if ( sb1 == 0 || s2 == 0 ) {
		return False;
	}

  Fixnum i;
	for(i=0; i<sb1->length; ++i) {
		if ( (s2[i] == 0) || (sb1->data[i] != s2[i]) ) {
			return False;
		}
	}
  if ( s2[i] == 0 ) { 
    return True;
  }
  else {
    return False;
  }
}

void string_truncate_buffer(Object *str, Fixnum len) {
  StringBuffer *buf = get_string_buffer(str);
  if ( buf != 0 && buf->reserved <= len ) {
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

  str->buffer = (Buffer *)nb;
  mem_free(cb);
}

// forward decl
void add_sym(Object *obj, char const* str);

void init_string_symbols() {
	add_sym(String, "String");
	add_sym(Parent, "parent");
	add_sym(True,   "True");
  add_sym(False,  "False");
}




