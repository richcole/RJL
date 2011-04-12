{ sys |

  test:: { x |
    while ( x <= 4 ) {
      x: (x + 1);
      if ( x >= 3 ) {
        return x;
      };
    }
    return 1;
  };
  println: (test: 0);
};
    