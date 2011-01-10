#define def_get_buffer(typeName, lcName)                         \
  typeName##Buffer *get_##lcName##_buffer(Object *obj) {         \
    do {                                                         \
	    if ( obj->buffer != 0 && obj->buffer->type == typeName ) { \
        return (typeName##Buffer *) obj->buffer;                 \
	    };                                                         \
	    obj = get(obj, Parent);                                    \
  	} while ( exists(obj) );                                     \
    return 0;                                                    \
  }
