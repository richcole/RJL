{ sys |

  Array each:: { block: |
    i: 0;
    while( i < (self length) ) {
      block: (self at: i);
      i: (i + 1);
    };
  };

  Array join:: { sep |
    result: "";
    first: true;
    self each: { x |
      if ( first ) {
        first: false;
      }
      else {
        result << sep;
      };
      result << (x to_s);
    };
    result;
  };

  a: (Array new);
  a push: 1;
  a push: 2;

  println: (a join: " ");
}
  