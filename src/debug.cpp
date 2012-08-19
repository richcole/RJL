#include "debug.h"
#include <stdio.h>

void trace(char const* msg) {
  fprintf(stdout, "TRACE %s\n", msg);
};


