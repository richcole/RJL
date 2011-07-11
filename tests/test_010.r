{ sys |

  BoxedInt: (sys BoxedInt);

  Range: (Object new: {
    self lower:upper:: { lower upper |
      Range new: {
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
    Range lower: self upper: upper;
  };

  (1 .. 10) each: { value |
    sys println: value;
  };
}