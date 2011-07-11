{ sys |

  b:: { x |
    raise: x;
  };

  a:: { x |
    try {
      b: x;
    }
    catch { ex |
      if ( (ex reason) >= 5 ) {
        ex reason;
      }
      else {
        11;
      };
    };
  };

  println: (a: 5);
};
