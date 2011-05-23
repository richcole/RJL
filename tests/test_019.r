{ sys |
  a:: { x | try { raise: x; } catch { ex | println: (ex reason); }; };
  a: "Hello World";
}
