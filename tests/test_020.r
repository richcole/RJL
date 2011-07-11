{ sys |

  b:: { x |
    raise: x;
  };

  a:: { x |
    try {
      b: x;
    }
    catch { ex |
      if ( ex >= 5 ) {
        1;
      }
      else {
        0;
      };
    };
  };

  println: (a: 5);
};
