
#include <stdio.h>
#include <errno.h>
#include <string.h>

struct FileBuffer {
	Object *type;
	FILE   *file;
};

Object *File = new_object();
Object *Errno = new_object();

FileBuffer* new_file_buffer(FILE *file) {
  FileBuffer *buf = (FileBuffer *)mem_alloc(sizeof(FileBuffer));
  buf->type = File;
  buf->file = file;
  return buf;
}

def_get_buffer(File, file);

Object *new_file(FILE *file) {
	Object *obj = new_object();
	obj->buffer = (Buffer *) new_file_buffer(file);
  return obj;
}

Fixnum is_file(Object *obj) {
	FileBuffer *file_buf = get_file_buffer(obj);
  if ( file_buf != 0 ) {
    return 1;
  }
  else {
    return 0;
  }
}

Object* native_file_open_mode(Object *frame, Object *self) {
	Object *stack = get(frame, Stack);
	CharArrayBuffer *file_mode_buf = get_char_array_buffer(pop(stack));
	CharArrayBuffer *file_name_buf = get_char_array_buffer(pop(stack));
	if ( file_name_buf == 0 ) {
		return new_exception(frame, "Expected a char_array as first argument");
	}
	if ( file_mode_buf == 0 ) {
		return new_exception(frame, "Expected a char_array as second argument");
	}
  Object *file_object = new_file(fopen(file_name_buf->data, file_mode_buf->data));
  set(file_object, Parent, self);
	push(stack, file_object);
	return frame;
}

Object* native_file_open(Object *frame, Object *self) {
	Object *stack = get(frame, Stack);
  push(stack, new_char_array("r"));
  return native_file_open_mode(frame, self);
}

Object *new_errno_exception(Object *frame, char const* message) {
  Object *ex = new_exception(frame, message);
  set(ex, Errno, new_char_array(strerror(errno)));
  return ex;
};

Object* native_file_close(Object *frame, Object *self) {
	FileBuffer *file_buf = get_file_buffer(self);
	if ( file_buf == 0 ) {
		return new_exception(frame, "Expected a file as first argument");
	}
	if ( file_buf->file == 0 ) {
		return new_exception(frame, "File pointer is null");
	}
	int result = fclose(file_buf->file);
	if ( result != 0 ) {
		return new_errno_exception(frame, "Failed while closing file");
	}
  return frame;
}

Object* native_file_eof(Object *frame, Object *self) {
	Object *stack = get(frame, Stack);
	FileBuffer *file_buf = get_file_buffer(self);
  if ( file_buf == 0 ) {
  	return new_exception(frame, "Expected a file as first argument");
  }
	if ( feof(file_buf->file) ) {
		push(stack, True);
	}
	else {
	  push(stack, False);
	}
	return frame;
};
Object* native_file_read_into_offset_length(Object *frame, Object *self) {
	Object *stack = get(frame, Stack);

	FileBuffer *file_buf = get_file_buffer(self);
  Object *length_obj   = pop(stack);
  Object *offset_obj   = pop(stack);
  Object *char_array       = pop(stack);

  if ( file_buf == 0 ) {
  	return new_exception(frame, "Expected a file as self argument");
  }

  CharArrayBuffer *char_array_buf = get_char_array_buffer(char_array);
  if ( char_array_buf == 0 ) {
  	return new_exception(frame, "Expected a char_array as first argument");
  }

  if ( ! is_fixnum(offset_obj) ) {
  	return new_exception(frame, "Expected a fixnum as second argument");
  }
  Fixnum offset = fixnum(offset_obj);
  
  if ( ! is_fixnum(length_obj) ) {
  	return new_exception(frame, "Expected a fixnum as third argument");
  }
  Fixnum length = fixnum(length_obj);

  if ( offset + length > char_array_buf->reserved ) {
  	return new_exception(frame, "Offset + length exceeds char_array length");
  }
  
  int num_read = fread(char_array_buf->data + offset, 1, length, file_buf->file);
  if ( num_read >= 0 ) {
    char_array_truncate_buffer(char_array, offset + num_read);
    return frame;
  }
  else {
  	return new_errno_exception(frame, "Unable to read from file");
  }
}

void init_file_symbols() {
	add_sym(File, "File");
	add_sym(Errno, "Errno");
}

void init_file_sys(Object *sys) {
	Object *file_object = new_object();
	set(sys, File, file_object);
	set(file_object, sym("open:"),                    new_func(native_file_open));
	set(file_object, sym("open:mode:"),               new_func(native_file_open_mode));
	set(file_object, sym("close"),                    new_func(native_file_close));
	set(file_object, sym("read:into:offset:length:"), new_func(native_file_read_into_offset_length));
	set(file_object, sym("eof"),                      new_func(native_file_eof));
}
