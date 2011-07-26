{ sys |

  try {

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
    |);
  
    valid:: { board |
      try {
        board rows each: { row_a |
          board rows each: { row_b |
            offset: (row_a row_index) - (row_b row_index);
            queen_offset: (row_a queen_index) - (row_b queen_index);
            if ( offset == queen_offset || queen_offset == 0 ) {
              raise: (| self invalid_board: true; |);
            };
          };
        };
        true;
      }
      catch { ex |
        if ( (ex reason invalid_board) ) {
          false;
        }
        else {
          raise: ex;
        };
      };
    };
    
    permutations:with:do:: { n array block: |
      println: ("array=" + (array join: ", "));
      if ((array length) == n) {
        block: array;
      }
      else {
        (1 .. n) each: { x |
          println: ("x= " + (x to_s) + ", array=" + (array join: " "));
          if ( not: (array contains: x) ) {
            array push: x;
            permutations: n with: array do: ^block:;
            array pop;
          };
        };
      };
    };
  
    permutations:do:: { n block |
      println: "Trace 2";
      permutations: n with: (Array new) do: ^block;
    };
  
    n_queens:: { n |
      println: "Trace 3";
      try {
        permutations: n do: { array |
          board: (| 
            self n: n;
            self rows: (array map_with_index: { index val |
              (| self row_index: index; self queen_index: val; |);
            });
          |);
          if ( valid: board ) {
            raise: (| self is_solution: true; self board: board; |);
          };
        };
        undefined;
      }
      catch { ex |
        if ( ex reason is_solution ) {
          ex reason board;
        }
        else {
          raise: ex;
        };
      };
    };
  
    solution: (n_queens: 5);
    if ( solution != undefined ) {
      println: ("Solution: " + ((solution rows map: { row |
        ((1 .. (solution n)) map: { x |
          if ( row queen_index == x ) {
            "q";
          }
          else {
            " ";
          };
        }) join: "";
      }) join: "\n"));
    }
    else {
      println: "No Solution";
    };
  }
  catch { ex |
    println: "Program failed";
    while ( ex reason ) {
      if ( ex frame ) {
        println: "** Frame ";
        dump: (ex frame) to: 6;
      };
      ex: (ex reason);
    };
    dump: ex to: 4;
  };
}
  