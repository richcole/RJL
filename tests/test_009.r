{ sys |
  a: (Object new: { self a: 1; self b: (self a); });
  println: (a b);
};
