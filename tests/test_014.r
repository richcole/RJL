{ sys |

  test:: { x |
    if ( x > 1 ) {
      if ( x > 2 ) {
        x;
      }
      else {
        0;
      };
    }
    else {
      0;
    };
  };

  println: (test: 10);
};