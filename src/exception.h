
Object *Exception = new_object();

Object *new_exception(Object *frame, char const* reason) {
  Object *obj = new_object();
  set(obj, Parent, Exception);
  set(obj, Frame,  frame);
  set(obj, Reason, new_string(reason));
}
