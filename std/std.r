{ sys |

  native_println:: (sys get_slot: #println:);

  sys println:: { s |
    native_println: (s to_s);
  };

  sys not:: { val |
    if ( val ) {
      false;
    }
    else {
      true;
    };
  };

  sys Array each:: { block: |
    i: 0;
    while( i < (self length) ) {
      block: (self at: i);
      i: (i + 1);
    };
  };

  sys Object merge:: { other |
    dest_obj: self;
    other slots each: { other_slot |
      if ( not: (other_slot == #parent )) {
        dest_obj set_slot: other_slot value: (other get_slot: other_slot);
      };
    };
  };

  sys Object ||:: { x |
    if ( self ) {
      self;
    }
    else {
      x;
    };
  };


  sys Enumerable: (|
    self map:: { block: |
      result: (Array new);
      self each: { x |
        result push: (block: x);
      };
      result;
    };

    self inject:func:: { zero func:value: |
      result: zero;
      self each: { x |
        result: (func: result value: x);
      };
      result;
    };

    self map_with_index:: { block:value: |
      i: 0;
      self map: { value |
        result: (block: i value: value);
        i: (i + 1);
        result;
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

    self to_s: {
      self join: ", ";
    };
  |);

  sys Array merge: (sys Enumerable);

  sys CharArray to_s: { self; };

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
      result: (sys Array new);
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
    permutations: n with: (sys Array new) do: ^block;
  };

};
