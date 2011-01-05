
struct StringBuffer {
  Object *type;
  Fixnum length;
  char   data[0];
};

Object *String = new_object();

Fixnum strlen(char const* s) {
  Fixnum i = 0;
  while(s[i] != 0) ++i;
  return i;
}

void memcpy(void *dst, void const* src, int len) {
  char *p = (char *)dst;
  char *q = (char *)src;
  while(len-- > 0) {
    *p++ = *q++;
  }
}

StringBuffer *new_string_buffer(char const* s, int len) {
  StringBuffer *buf = (StringBuffer *)mem_alloc(sizeof(StringBuffer)+len+1);
  buf->type      = String;
  buf->length    = len+1;
  buf->data[len] = 0;
  memcpy(buf->data, s, len);
  return buf;
};

StringBuffer *new_string_buffer(char const* s) {
  return new_string_buffer(s, strlen(s));
}

Object *new_string(char const* s) {
  Object *obj = new_object();
  obj->buffer = (Buffer *) new_string_buffer(s);
  return obj;
}

def_get_buffer(StringBuffer, string);

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

// forward decl
void add_sym(Object *obj, char const* str);

void init_string_symbols() {
	add_sym(String, "String");
}




