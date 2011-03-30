{ sys |
  BoxedInt: (sys BoxedInt);

  Range: (|
    lower:upper:: { lower upper |
      result: (Range clone);
      result lower: lower;
      result upper: upper;
      return result;
    };

    each:: { block |
      i: (self lower);
      while ( i <= (self upper) ) {
        block call: #nil with: i;
        i: (i + 1);
      };
    };
  |);

  BoxedInt ..:: { upper |
    return Range lower: self upper: upper;
  };

  (1 .. 10) each: { value |
    sys println: value;
  };
}