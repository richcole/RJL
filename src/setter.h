Fixnum is_nocall_slot(Object *value) {
  Fixnum len = string_length(value);
  if ( len > 0 ) {
    return string_get_at(value, 0) == '^';
  }
  else {
    return 0;
  }
}

Object* get_nocall_slot(Object *slot) {
  Fixnum len = string_length(slot);
  if ( len > 0 ) {
    return sym(string_substring(slot, 1, len));
  }
  else {
    return Undefined;
  }
}

Fixnum is_setter_slot(Object *value) {
  Fixnum len = string_length(value);
  if ( len > 0 ) {
    return string_get_at(value, len-1) == ':';
  }
  else {
    return 0;
  }
}

Object* setter_field(Object *value) {
  Fixnum len = string_length(value);
  if ( len > 0 ) {
    return sym(string_substring(value, 0, len-1));
  }
  else {
    return Undefined;
  }
}
