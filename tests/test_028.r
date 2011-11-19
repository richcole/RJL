{ sys |

  sys load: "std/std.r";

  a: (Array new); 
  a push: 1;
  a push: 2;
  a push: 3;

  println: (a inject: 0 func: { x y | x + y; });
};