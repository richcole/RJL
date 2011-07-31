{ sys |

  sys Object ||:: { x |
    if ( self ) {
      self;
    }
    else {
      x;
    };
  };

  sys Array map:: { block: |
    result: (Array new);
    self each: { x |
      result push: (block: x);
    };
    result;
  };

  sys CharArray to_s: { self; };

  sys Array map_with_index:: { block:value: |
    result: (Array new);
    i: 0;
    while( i < (self length) ) {
      value: (block: i value: (self at: i));
      result push: value;
      i: (i + 1);
    };
    result;
  };

  sys Array to_s: {
    self join: ", ";
  };

  sys Array each:: { block: |
    i: 0;
    while( i < (self length) ) {
      block: (self at: i);
      i: (i + 1);
    };
  };

  sys Array join:: { sep |
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

  sys not:: { val |
    if ( val ) {
      false;
    }
    else {
      true;
    };
  };

  sys Array contains:: { key |
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

  sys BoxedInt ..:: { upper |
    Range lower: self upper: upper;
  };

  sys Range: (| 
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

  sys permutations:with:do:: { n array block: |
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

  sys permutations:do:: { n block |
    permutations: n with: (Array new) do: ^block;
  };
};
