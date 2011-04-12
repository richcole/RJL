{ sys |

  b: (|
    self do:: { blk |
      ^blk call;
    };
  |);
  a: (|
    self test:: { b | 
      b do: { sys println: (self value); }; 
    };
    self value: "Hello World"; 
  |);
  a test: b;
};