#include <string.h>
#include <stdio.h>

#include "code_test.h"
#include "rjl.h"
#include "scanner.h"

int main(int argc, char **argv) {
  if ( argc < 2 ) {
    fprintf(stderr, "Expected arguments\n");
  }
  if ( strcmp(argv[1], "test") == 0 ) {
    code_test_execute_tests();
  }
  if ( strcmp(argv[1], "scan") == 0 ) {
    if ( argc < 3 ) {
      fprintf(stderr, "Expected arguments\n");
    }
    cxt_t *cxt = new_cxt();
    scanner_scan(cxt, argv[2]);
  }
}
