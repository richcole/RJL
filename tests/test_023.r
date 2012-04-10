{ sys |

  sys load: "std/std.r";

  board_to_s:: { board |
    ((solution rows map: { row |
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

  valid:: { board |
    catch_throw: {
      board rows each: { row_a |
        board rows each: { row_b |
          if ((row_b row_index) > (row_a row_index)) {
            offset: ((row_a row_index) - (row_b row_index));
            queen_offset: ((row_a queen_index) - (row_b queen_index));
            if ( (offset == queen_offset) || (queen_offset == 0) ) {
              throw: false;
            };
          };
        };
      };
      true;
    };
  };
    
  n_queens:: { n |
    catch_throw: {
      permutations: n do: { array |
        board: (| 
          self n: n;
          self rows: (array map_with_index: { index val |
            (| self row_index: index; self queen_index: val; |);
          });
        |);
        if ( valid: board ) {
          throw: board;
        };
      };
      undefined;
    };
  };
  
  solution: (n_queens: 5);

  if ( solution == undefined ) {
    println: "No Solution";
  }
  else {
    println: "Solution:";
    println: (board_to_s: solution);
  };
};
