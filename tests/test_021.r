{ sys |

  raise: "Loops forever";

  valid:: { board |
    InvalidBoard: Object new;
    try {
      board rows each: { row_a |
        board rows each: { row_b |
          offset: (row_a row_index) - (row_b row_index);
          queen_offset: (row_a queen_index) - (row_b queen_index);
          if ( offset == queen_offset || queen_offset == 0 ) {
            raise: (| invalid_board: true; |);
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
        raise ex;
      };
    };
  }
  
  permutations:with:do:: { n array block: |
    if ((array size) == n) {
      block: array;
    }
    else {
      (1 .. n) each: { x |
        if ( not: (array contains: x) ) {
          array push: x;
          permutations: n with: array do: block:;
          array pop;
        };
      };
    };
  };

  permutations:do:: { n block |
    permutations: n with: (Array new) do: block;
  };

  n_queens:: { n |
    try {
      permutations: n do: { array |
        board: (| 
          n: n;
          rows: (array map_with_index: { index val |
            (| row_index: index; queen_index: val; |);
          });
        |);
        if ( valid: board ) {
          raise: (| is_solution: true; board: board; |)
        };
      };
      undefined;
    }
    catch { ex |
      if ( ex reason is_solution ) {
        ex reason board;
      }
      else {
        raise ex;
      }
    }
  }

  solution: n_queens 5;
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
