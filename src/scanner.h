#ifndef RJL_SCANNER_H
#define RJL_SCANNER_H

#include "rjl.h"

fixnum new_file(cxt_t *cxt, fixnum filename);
fixnum new_scanner(cxt_t *cxt, fixnum file);
fixnum scanner_next_token(cxt_t *cxt, fixnum scanner);
void scanner_scan(cxt_t *cxt, char const* filename);

#endif
