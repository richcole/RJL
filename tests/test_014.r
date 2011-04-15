{ sys |

  test:: { x |
    if ( x > 1 ) {
      if ( x > 2 ) {
        return x;
      };
    };
    return 0;
  };

  println: (test: 10);
};