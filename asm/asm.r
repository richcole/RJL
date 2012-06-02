{ sys |

  load: "std/std.r";
  
  output: (File open: "a.out" mode: "w");
  try {
    buf: (CharArray new);
    buf: "Hello World";
  }
  finally {
  }

  
  
};