{ sys |
  b = 1;
  obj = (| a b = 1; b = 2; a b = 2; b = 3 |);
  sys print (obj b);
  sys print (obj a b);
  sys print "\n";
}

