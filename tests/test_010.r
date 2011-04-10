{ sys |
  sys Object new:: { block |
    result: Object new;
    ^block invoke: result;
    return result;
  };

  BoxedInt: (sys BoxedInt);

  Range: (Object new: {
    self lower:upper:: { lower upper |
      return Range new: {
        self lower: lower;
        self upper: upper;
      };
    };

    self each:: { block |
      i: (self lower);
      while ( i <= (self upper) ) {
        ^block call: i;
        i: (i + 1);
      };
    };
  });

  BoxedInt ..:: { upper |
    return Range lower: self upper: upper;
  };

  (1 .. 10) each: { value |
    sys println: value;
  };
}