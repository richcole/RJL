{ sys |

  fib:: { x | 
    if ( x <= 1 ) {
      return x;
    };
    return (fib: (x - 1)) + (fib: (x - 2));
  };

  println: (fib: 10);
};