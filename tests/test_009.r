{ sys |
  sys Object new:: { block |
    result: Object new;
    ^block invoke: result;
    return result;
  };
  a: (Object new: { self a: 1; self b: (self a); });
  println: (a b);
};
