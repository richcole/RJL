#ifndef BUFFER_H
#define BUFFER_H

#define decl_get_buffer(typeName, lcName, type_tag)                        \
  typeName##Buffer *get_##lcName##_buffer(Object *obj);

#define decl_set_buffer(typeName, lcName, type_tag)                        \
  void set_##lcName##_buffer(Object *obj, typeName##Buffer* buf);

#define def_get_buffer(typeName, lcName, type_tag)                         \
  typeName##Buffer *get_##lcName##_buffer(Object *obj) {                   \
    if ( obj != 0 && obj->buffer != 0 && obj->buffer->type == type_tag) {              \
      return (typeName##Buffer *) obj->buffer;                             \
    };                                                                     \
    return 0;                                                              \
  }

#define def_set_buffer(typeName, lcName, type_tag)                              \
  void set_##lcName##_buffer(Object *obj, typeName##Buffer* buf) {              \
    if ( obj->buffer != 0 && obj->buffer->type == type_tag ) {                  \
      obj->buffer = (Buffer *) buf;                                             \
    };                                                                          \
  }                                                                             \

#define create_new_buffer(typeName, lcName, type_tag, extra_mem)                \
  Fixnum mem_size  = sizeof(typeName##Buffer)+(extra_mem);                      \
  typeName##Buffer *buf = (typeName##Buffer *)context_alloc_buffer(cxt, mem_size);     \
  buf->type = type_tag;                                                         \

#define def_new_buffer(typeName, lcName, type_tag, fieldType, fieldName)        \
  typeName##Buffer *new_##lcName##_buffer(Object *cxt, fieldType fieldName) {   \
    create_new_buffer(typeName, lcName, type_tag, 0);                           \
    buf->fieldName = fieldName;                                                 \
    return buf;                                                                 \
  }                                                                             \

#define decl_new_buffer(typeName, lcName, type_tag, fieldType, fieldName)       \
  typeName##Buffer *new_##lcName##_buffer(Object *cxt, fieldType fieldName);    

#define def_is_buffer(typeName, lcName, type_tag)         \
  Fixnum is_##lcName(Object *obj) {                       \
    typeName##Buffer *buf = get_##lcName##_buffer(obj);   \
    return buf != 0;                                      \
  }                                                       \

#define decl_is_buffer(typeName, lcName, type_tag)       \
  Fixnum is_##lcName(Object *cxt, Object *obj);          \

#endif
