{ sys |

  Object ||:: { x |
    if ( self ) {
      self;
    }
    else {
      x;
    };
  };

  Array map:: { block: |
    result: (Array new);
    self each: { x |
      result push: (block: x);
    };
    result;
  };

  CharArray to_s: { self; };

  Array map_with_index:: { block:value: |
    result: (Array new);
    i: 0;
    while( i < (self length) ) {
      value: (block: i value: (self at: i));
      result push: value;
      i: (i + 1);
    };
    result;
  };

  Array to_s: {
    self join: ", ";
  };

  Array each:: { block: |
    i: 0;
    while( i < (self length) ) {
      block: (self at: i);
      i: (i + 1);
    };
  };

  Array join:: { sep |
    result: (CharArray new);
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

  not:: { val |
    if ( val ) {
      false;
    }
    else {
      true;
    };
  };

  Array contains:: { key |
    try {
      self each: { x |
        if ( x == key ) {
          raise: true;
        };
      };
      false;
    }
    catch { ex |
      if ( ex reason == true ) {
        true;
      }
      else {
        raise: ex;
      };
    };
  };

  BoxedInt ..:: { upper |
    Range lower: self upper: upper;
  };

  Range: (| 
    self lower:upper:: { lower upper |
      self new: {
        self lower: lower;
        self upper: upper;
      };
    };

    self each:: { block: |
      i: (self lower);
      while ( i <= (self upper) ) {
        block: i;
        i: (i + 1);
      };
    };

    self map:: { block: |
      result: (Array new);
      self each: { x |
        result push: (block: x);
      };
      result;
    };
  |);

  permutations:with:do:: { n array block: |
    if ((array length) == n) {
      block: array;
    }
    else {
      (1 .. n) each: { x |
        if ( not: (array contains: x) ) {
          array push: x;
          permutations: n with: array do: ^block:;
          array pop;
        };
      };
    };
  };

  permutations:do:: { n block |
    permutations: n with: (Array new) do: ^block;
  };
};