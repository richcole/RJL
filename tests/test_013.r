{ sys |

  test:: { x |
    while ( x <= 4 ) {
      x: (x + 1);
    };
  };
  println: (test: 0);
};
    