{ sys |

  sys Range = (| 
    each = { func |
      index = self lower;
      while ( index <= (self upper) ) {
        func index;
        index = index + 1;
      };
    };
  |);

  sys Integer .. = { upper | 
    lower = self;
    range = sys Range new;
    range lower = self;
    range upper = upper;
    return range;
  };

  ( 1 .. 10 ) each { x | sys print x; };
};