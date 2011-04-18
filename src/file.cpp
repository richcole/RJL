#include "file.h"
#include "std.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

struct FileBuffer {
	Object *type;
	FILE   *file;
};

def_new_buffer(File, file, FILE *, file);
def_get_buffer(File, file);
def_set_buffer(File, file);
def_is_buffer(File, file);

Object *new_file(Object *cxt, FILE *file) {
  Object *obj = new_object(cxt);
  obj->buffer = (Buffer *) new_file_buffer(cxt, file);
  return obj;
}

Object* native_file_open_mode(Object *cxt, Object *frame, Object *self) {
  Object *stack = get(cxt, frame, "stack");
  CharArrayBuffer *file_mode_buf = get_char_array_buffer(cxt, pop(cxt, stack));
  CharArrayBuffer *file_name_buf = get_char_array_buffer(cxt, pop(cxt, stack));
  if ( file_name_buf == 0 ) {
    return new_exception(cxt, frame, "Expected a char_array as first argument");
  }
  if ( file_mode_buf == 0 ) {
    return new_exception(cxt, frame, "Expected a char_array as second argument");
  }
  Object *file_object = new_file(cxt, fopen(file_name_buf->data, file_mode_buf->data));
  set(cxt, file_object, "parent", self);
  push(cxt, stack, file_object);
  return frame;
}

Object* native_file_open(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  push(cxt, stack, new_char_array(cxt, "r"));
  return native_file_open_mode(cxt, frame, self);
}

Object *new_errno_exception(Object *cxt, Object *frame, char const* message) {
  Object *ex = new_exception(cxt, frame, message);
  set(cxt, ex, "error", new_char_array(cxt, strerror(errno)));
  return ex;
};

Object* native_file_close(Object *cxt, Object *frame, Object *self) {
  FileBuffer *file_buf = get_file_buffer(cxt, self);
  if ( file_buf == 0 ) {
    return new_exception(cxt, frame, "Expected a file as first argument");
  }
  if ( file_buf->file == 0 ) {
    return new_exception(cxt, frame, "File pointer is null");
  }
  int result = fclose(file_buf->file);
  if ( result != 0 ) {
    return new_errno_exception(cxt, frame, "Failed while closing file");
  }
  return frame;
}

Object* native_file_eof(Object *cxt, Object *frame, Object *self) {
  Object *stack = get(cxt, frame, "stack");
  FileBuffer *file_buf = get_file_buffer(cxt, self);
  if ( file_buf == 0 ) {
  	return new_exception(cxt, frame, "Expected a file as first argument");
  }
  if ( feof(file_buf->file) ) {
    push(cxt, stack, "true");
  }
  else {
    push(cxt, stack, "false");
  }
  return frame;
};

Object* native_file_read_into_offset_length(Object *cxt, Object *frame, Object *self) {
  Object *stack = get_stack(cxt, frame);
  
  FileBuffer *file_buf = get_file_buffer(cxt, self);
  Object *length_obj   = pop(cxt, stack);
  Object *offset_obj   = pop(cxt, stack);
  Object *char_array   = pop(cxt, stack);

  if ( file_buf == 0 ) {
  	return new_exception(cxt, frame, "Expected a file as self argument");
  }

  CharArrayBuffer *char_array_buf = get_char_array_buffer(cxt, char_array);
  if ( char_array_buf == 0 ) {
  	return new_exception(cxt, frame, "Expected a char_array as first argument");
  }

  if ( ! is_boxed_int(cxt, offset_obj) ) {
  	return new_exception(cxt, frame, "Expected a boxed int as second argument");
  }
  Fixnum offset = boxed_int_to_fixnum(cxt, offset_obj);
  
  if ( ! is_boxed_int(cxt, length_obj) ) {
  	return new_exception(cxt, frame, "Expected a fixnum as third argument");
  }
  Fixnum length = boxed_int_to_fixnum(cxt, length_obj);

  if ( offset + length > char_array_buf->reserved ) {
  	return new_exception(cxt, frame, "Offset + length exceeds char_array length");
  }
  
  int num_read = fread(char_array_buf->data + offset, 1, length, file_buf->file);
  if ( num_read >= 0 ) {
    char_array_truncate_buffer(cxt, char_array, offset + num_read);
    return frame;
  }
  else {
  	return new_errno_exception(cxt, frame, "Unable to read from file");
  }
}

void init_file_sys(Object *cxt) {
  Object *file_object = context_get(cxt, "File");
  set(cxt, file_object, "open:",                    new_func(cxt, native_file_open));
  set(cxt, file_object, "open:mode:",               new_func(cxt, native_file_open_mode));
  set(cxt, file_object, "close",                    new_func(cxt, native_file_close));
  set(cxt, file_object, "read:into:offset:length:", new_func(cxt, native_file_read_into_offset_length));
  set(cxt, file_object, "eof",                      new_func(cxt, native_file_eof));
}
