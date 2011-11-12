{ sys |

  sys Object merge: {

  sys Object ||:: { x |
    if ( self ) {
      self;
    }
    else {
      x;
    };
  };

  sys Enumerable merge: (|
    self map:: { block: |
      result: (Array new);
      self each: { x |
        result push: (block: x);
      };
      result;
    };

    self map_with_index:: { block:value: |
      i: 0;
      self map: { value |
        block: i value: value;
        i: (i + 1);
      };
    };

    self join:: { sep |
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

    self contains:: { key |
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
  };

  sys Array merge: Enumerable;

  sys CharArray to_s: { self; };

  sys Array merge: (|
    self to_s: {
      self join: ", ";
    };

    self each:: { block: |
      i: 0;
      while( i < (self length) ) {
        block: (self at: i);
        i: (i + 1);
      };
    };
  |);

  sys not:: { val |
    if ( val ) {
      false;
    }
    else {
      true;
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
