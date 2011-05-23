{ sys |

  b:: {
    raise: x;
  };

  a:: { x |
    try {
      b: x;
    }
    catch { ex |
      if ( x > 5 ) {
        return x;
      }
      else {
        return 1;
      };
    };
  };

  println: (a: 5);
};
