{ sys |

  Array: (sys Array);

  Array each:: { block: |
    index: 0;
    while ( index < (self length) ) {
      block: (self at: index);
      index: (index + 1);
    };
  };

  xs: (Array new);
  xs push: 1;
  xs push: 2;

  xs each: { x | println: x; };
};
    