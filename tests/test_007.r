{ sys |
  a: (| 
    test: {
      self do: { sys println: (self value); }; 
    };
    value: "Hello World"; 
    do:: { blk |
      blk call: self;
    };
  |);
  a test;
};