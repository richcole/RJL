
Object *Exception = new_object();

Object *find_catch_frame(Object *frame) {
  while( exists(frame) ) {
    if ( array_length(get(frame, Catch)) != 0 ) {
      return frame;
    }
    frame = get(frame, Parent);
  }
  return Nil;
}

Object *new_exception_frame(Object *frame, Object *ex) {
  Object *catch_frame = new_object(find_catch_frame(frame));
  push(get(catch_frame, Stack), ex);
  set(catch_frame, Pc, pop(get(catch_frame, Catch)));
  return catch_frame;
}

Object *new_exception(Object *frame, char const* reason) {
  Object *ex = new_object();
  set(ex, Parent, Exception);
  set(ex, Frame,  frame);
  set(ex, Reason, new_string(reason));

  return new_exception_frame(frame, ex);
}

void init_exception_symbols() {
	add_sym(Exception, "Exception");
}
