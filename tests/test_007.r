{ sys |
  sys Object new:: { block |
    result: Object new;
    ^block invoke: result;
    return result;
  };

  b: (Object new: {
    self do:: { blk |
      ^blk call;
    };
  });
  a: (Object new: { 
    self test:: { b | 
      b do: { sys println: (self value); }; 
    };
    self value: "Hello World"; 
    return self;
  });
  a test: b;
};