struct BoxedIntBuffer {
  Object *type;
  long   value;
};

BoxedIntBuffer *new_boxed_int_buffer(long value) {
  BoxedIntBuffer *buf = (BoxedIntBuffer *)mem_alloc(sizeof(BoxedIntBuffer));
  buf->type   = BoxedInt;
  buf->value  = value;
  return buf;
}

Object *new_boxed_int(Object ) {
  Object *boxed_int = new_object(cxt );
  boxed_int->buffer = (Buffer *) new_boxed_int_buffer(0);
  set(boxed_int, Parent, BoxedIntObject);
  return boxed_int;
}

Object *new_boxed_int(long value) {
  Object *boxed_int = new_object();
  boxed_int->buffer = (Buffer *) new_boxed_int_buffer(value);
  set(boxed_int, Parent, BoxedIntObject);
  return boxed_int;
}

def_get_buffer(BoxedInt, boxed_int);
def_set_buffer(BoxedInt, boxed_int);

Fixnum is_boxed_int(Object *boxed_int) {
  if ( get_boxed_int_buffer(boxed_int) != 0 ) {
    return 1;
  }
  return 0;
}

Object *char_array_to_boxed_number(Object *str) {
  Object *boxed_int = new_boxed_int();
  BoxedIntBuffer *int_buf = get_boxed_int_buffer(boxed_int);
  CharArrayBuffer   *str_buf = get_char_array_buffer(str);
  Fixnum sign = 1;
  Fixnum i = 0;

  if ( str_buf == 0 || int_buf == 0 ) {
    return Undefined;
  }
  if ( str_buf->data[i] == '-' ) {
    sign = -1;
    ++i;
  }
  while( str_buf->data[i] >= '0' && str_buf->data[i] <= '9' ) {
    int_buf->value *= 10;
    int_buf->value += (str_buf->data[i] - '0');
    ++i;
  }
  int_buf->value *= sign;

  return boxed_int;
}

