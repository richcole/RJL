{ sys |

  sys load: "std/std.r";

  a: (catch_throw: { 
      ( 1 .. 5 ) each: { x |
        ( 1 .. 5 ) each: { y |
          throw: (x + y);
        };
      };
    });

  println: a;
};
