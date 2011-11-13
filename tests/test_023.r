{ sys |

  sys load: "std/std.r";

  valid:: { board |
    try {
      board rows each: { row_a |
        board rows each: { row_b |
          if ((row_b row_index) > (row_a row_index)) {
            offset: ((row_a row_index) - (row_b row_index));
            queen_offset: ((row_a queen_index) - (row_b queen_index));
            if ( (offset == queen_offset) || (queen_offset == 0) ) {
              raise: (| self invalid_board: true; |);
            };
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

  n_queens:: { n |
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

  if ( solution == undefined ) {
    println: "No Solution";
  }
  else {
    println: "Solution:";
    println: ((solution rows map: { row |
      ((1 .. (solution n)) map: { x |
        if ( (row queen_index) == x ) {
          "q";
        }
        else {
          ".";
        };
      }) join: "";
    }) join: "\n");
  };
}
  