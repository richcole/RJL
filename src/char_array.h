#ifndef CHAR_ARRAY_H
#define CHAR_ARRAY_H

#include "object.h"
#include "type_tags.h"

struct CharArrayBuffer {
  Fixnum type;
  Fixnum reserved;
  Fixnum length;
  char   data[0];
};

Object *new_char_array(Object *cxt, char const* s);
Object *new_char_array(Object *cxt, Fixnum reserved);
Fixnum  is_char_array(Object *cxt, Object *obj);
Fixnum  char_array_equals(Object *s1, Object *s2);
Fixnum  char_array_equals(Object *s1, char const* s2);
void    char_array_truncate_buffer(Object *cxt, Object *str, Fixnum len);
void    char_array_set_reserve(Object *cxt, Object *str, Fixnum new_size);
Fixnum  char_array_length(Object *cxt, Object *str);
Fixnum  char_array_reserve(Object *cxt, Object *str);
void    char_array_append(Object *cxt, Object *str, Object *arg);
Object* char_array_concat(Object *cxt, Object *str, Object *arg);
Object* char_array_subchar_array(Object *cxt, Object *char_array, Fixnum start, Fixnum end);
char    char_array_get_at(Object *cxt, Object *str, Fixnum index);
CharArrayBuffer *new_char_array_buffer(Object *cxt, Fixnum len);
CharArrayBuffer *new_char_array_buffer(Object *cxt, char const* s);
Object *new_char_array(char const* s);

decl_get_buffer(CharArray, char_array, CharArrayTypeTag);

#endif
