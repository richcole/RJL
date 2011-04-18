#ifndef BUFFER_H
#define BUFFER_H

#define decl_get_buffer(typeName, lcName)                           \
  typeName##Buffer *get_##lcName##_buffer(Object *cxt, Object *obj);

#define decl_set_buffer(typeName, lcName)                           \
  void set_##lcName##_buffer(Object *cxt, Object *obj, typeName##Buffer* buf);

#define def_get_buffer(typeName, lcName)                                   \
  typeName##Buffer *get_##lcName##_buffer(Object *cxt, Object *obj) {      \
    if ( obj == 0 ) return 0;                                              \
    do {                                                                   \
      if ( obj->buffer != 0 && obj->buffer->type == sym(cxt, #typeName)) { \
        return (typeName##Buffer *) obj->buffer;                           \
      };                                                                   \
      obj = get(cxt, obj, "parent");                                       \
  	} while ( exists(cxt, obj) );                                          \
    return 0;                                                              \
  }

#define def_set_buffer(typeName, lcName)                                        \
  void set_##lcName##_buffer(Object *cxt, Object *obj, typeName##Buffer* buf) { \
    do {                                                                        \
      if ( obj->buffer != 0 && obj->buffer->type == sym(cxt, #typeName) ) {     \
        obj->buffer = (Buffer *) buf;                                           \
        return;                                                                 \
      };                                                                        \
      obj = get(cxt, obj, "parent");                                            \
  	} while ( exists(cxt, obj) );                                               \
  }                                                                             \

#define create_new_buffer(typeName, lcName, extra_mem)                          \
  Fixnum mem_size  = sizeof(typeName##Buffer)+(extra_mem);                      \
  typeName##Buffer *buf = (typeName##Buffer *)mem_alloc(mem_size);              \
  buf->type   = sym(cxt, #typeName);                                            \

#define def_new_buffer(typeName, lcName, fieldType, fieldName)                  \
  typeName##Buffer *new_##lcName##_buffer(Object *cxt, fieldType fieldName) {   \
    create_new_buffer(typeName, lcName, 0);                                     \
    buf->fieldName = fieldName;                                                 \
    return buf;                                                                 \
  }                                                                             \

#define decl_new_buffer(typeName, lcName, fieldType, fieldName)                 \
  typeName##Buffer *new_##lcName##_buffer(Object *cxt, fieldType fieldName);    

#define def_is_buffer(typeName, lcName) \
  Fixnum is_##lcName(Object *cxt, Object *obj) { \
    typeName##Buffer *buf = get_##lcName##_buffer(cxt, obj); \
    if ( buf != 0 ) { \
      return 1; \
    } \
    else { \
      return 0; \
    } \
  } \

#define decl_is_buffer(typeName, lcName) \
  Fixnum is_##lcName(Object *cxt, Object *obj);



#endif
