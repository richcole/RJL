#ifndef FILE_H
#define FILE_H

#include "object.h"

decl_is_buffer(typeName, lcName, FileTypeTag);
decl_is_buffer(File, file, FileTypeTag);

void init_file_sys(Object *cxt);

#endif
