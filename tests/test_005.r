{ sys |
  sys Object new:: { block |
    result: Object new;
    ^block invoke: result;
    return result;
  };

  object: ( Object new: { self a: 1; self b: 2; } );
  sys println: (object a);
  sys println: (object b);
}
