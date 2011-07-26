{ sys |
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
    |);

    (1 .. 2) each: { x |
      println: x;
    };
};