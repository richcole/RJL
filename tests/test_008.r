{ sys |
  b: 2;
  a: Object new: { self a: 1; self b: b; };
  println: b;
};