{ sys |

  sys load: "std/std.r";
  sys load: "std/stream.r";
  
  stream: (FileStream open: "tests/input/test_029.input");

  while ( not: (stream eof) ) {
    println: (stream read);
  };
};