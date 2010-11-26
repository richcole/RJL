{ sys |
  fib = { x |
    if ( x <= 1 ) {
      return x;
    };
    i = 0;
    j = 1;
    index = 1; 
    while ( index < x ) {
      tmp = j;
      j = j + i;
      i = tmp;
      index = index + 1;
    };
    return j;
  };
  sys print (fib 6);
}

    