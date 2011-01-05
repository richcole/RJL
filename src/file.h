
#include <stdio.h>

struct FileBuffer {
	Object *type;
	FILE   *file;
};

Object *File = new_object();

FileBuffer* new_file_buffer(FILE *file) {
  FileBuffer *buf = (FileBuffer *)mem_alloc(sizeof(FileBuffer));
  buf->type = File;
  buf->file = file;
  return buf;
}

def_get_buffer(FileBuffer, file);

Object *new_file(FILE *file) {
	Object *obj = new_object();
	obj->buffer = (FileBuffer) new_file_buffer(file);
}

Object* native_file_open(Object *frame) {
	Object *stack = get(frame, Stack);
	StringBuffer file_name_buf = get_string_buffer(pop(stack));
	StringBuffer file_mode_buf = get_string_buffer(pop(stack));
	if ( file_name_buf == 0 ) {
		return new_exception(frame, "Expected a string as first argument");
	}
	if ( file_mode_buf == 0 ) {
		return new_exception(frame, "Expected a string as second argument");
	}
	push(stack, new_file(fopen(file_name_buf->data, file_mode_buf->data)));
	return frame;
}

Object* native_file_close(Object *close) {
	Object *stack = get(frame, Stack);
	FileBuffer *file = get_file_buffer(pop(stack));
	if ( file == 0 ) {
		return new_exception(frame, "Expected a file as first argument");
	}
	if ( file->file == 0 ) {
		return new_exception(frame, "File pointer is null");
	}
	int result = fclose(file->file);
	if ( result != 0 ) {
		return new_errno_exception(frame, "Failed while closing file");
	}
}

Object* native_file_eof(Object *frame) {
	Object *stack = get(frame, Stack);
	FileBuffer *file = get_file_buffer(pop(stack));
  if ( file == 0 ) {
  	return new_exception(frame, "Expected a file as first argument");
  }
	if ( feof(frame) ) {
		push(stack, True);
	}
	else {
	  push(stack, False);
	}
	return frame;
};

Object* native_file_read(Object *frame) {
	Object *stack = get(frame, Stack);

	FileBuffer *file = get_file_buffer(pop(stack));
  if ( file == 0 ) {
  	return new_exception(frame, "Expected a file as first argument");
  }

  Object *string = pop(stack);
  Object *string_buf = get_string_buffer(string);
  if ( string_buf == 0 ) {
  	return new_exception(frame, "Expected a string as second argument");
  }

  Object *offset_obj = pop(stack);
  if ( ! is_fixnum(offset_obj) ) {
  	return new_exception(frame, "Expected a fixnum as fourth argument");
  }
  Fixnum offset = fixnum(offset_obj);
  
  Object *length_obj = pop(stack);
  if ( ! is_fixnum(length_obj) ) {
  	return new_exception(frame, "Expected a fixnum as third argument");
  }
  Fixnum length = fixnum(length_obj);

  if ( offset + length >= string_buf->length ) {
  	return new_exception(frame, "Offset + length exceeds string length");
  }
  
  int num_read = fread(file->file, length, 1, string_buf->buf + offset);
  if ( num_read >= 0 ) {
  	(result_string->buf + offset)[num_read] = 0;
  	set(result_string, Tail, object(offset + num_read));  	
  	return frame;
  }
  else {
  	mem_free(result);
  	return new_errno_exception(frame, "Unable to read from file");
  }
}

void init_file_symbols() {
	add_sym(File, "File");
}

void init_file_sys(Object *sys) {
	Object *file_object = new_object();
	set(sys, File, file_object);
	set(file_object, sym("open:mode:"),              new_func(native_file_open));
	set(file_object, sym("close"),                   new_func(native_file_close));
	set(file_object, sym("read:into:offset:length:", new_func(native_file_read));
	set(file_object, sym("eof",                      new_func(native_file_eof));
}
