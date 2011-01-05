#define def_get_buffer(typeName, lcName) \
  typeName *get_##lcName##_buffer(Object *obj) { \
  	while ( obj != 0 ) { \
	    if ( obj->buffer != 0 && obj->buffer->type == typeName ) { \
	      return (typeName *) obj->buffer; \
	    } \
	    obj = get(obj, Parent); \
  	} \
    return 0; \
  }
