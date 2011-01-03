
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

FileBuffer *get_file_buffer(Object *obj) {
  if ( obj != 0 && obj->buffer != 0 && obj->buffer->type == File ) {
    return (FileBuffer *) obj->buffer;
  }
  return 0;
}

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

Object* native_file_read(Object *frame) {
	Object *stack = get(frame, Stack);
	FileBuffer *file = get_file_buffer(pop(stack));
  if ( file == 0 ) {
  	return new_exception(frame, "Expected a file as first argument");
  }
  Object *length_obj = pop(stack);
  if ( ! is_fixnum(length_obj) ) {
  	return new_exception(frame, "Expected a fixnum as second argument");
  }
  Fixnum length = fixnum(length_obj);
  Object *result = new_string(length);
  StringBuffer *result_string = get_string_buffer(result);
  int num_read = fread(file->file, length, 1, result_string->buf);
  if ( num_read >= 0 ) {
  	result_string->buf[num_read] = 0;
  	set(result_string, Tail, object(num_read));
  	push(stack, result);
  	return frame;
  }
  else {
  	mem_free(result);
  	return new_errno_exception(frame, "Unable to read from file");
  }
}

void init_file_symbols() {
	File->buffer = (Buffer *) new_string_buffer("File");
}

void init_file_sys(Object *sys) {
	Object *file_object = new_object();
	set(sys, File, file_object);
	set(file_object, Open, new_func(native_file_open));
	set(file_object, Close, new_func(native_file_close));
	set(file_object, Close, new_func(native_file_read));
}
