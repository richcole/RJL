{ sys |

  fib:: { x | 
    if ( x <= 1 ) {
      x;
    }
    else {
      (fib: (x - 1)) + (fib: (x - 2));
    };
  };

  println: (fib: 10);
};