#define def_get_buffer(typeName, lcName)                 \
  typeName##Buffer *get_##lcName##_buffer(Object *obj) { \
    while ( obj != 0 ) { \
	    if ( obj->buffer != 0 && obj->buffer->type == typeName ) { \
        	      return (typeName##Buffer *) obj->buffer;               \
	    }; \
	    obj = get(obj, Parent); \
  	} \
    return 0; \
  }
