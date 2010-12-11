= maximal_subtree

This program is another example of literate programming. It defines a
module with a single function maximal_subree. This function find the
node with the maximal sum.

  (| 
    types = { types, module |
      a_tree     = types an_instance of (types tree);
      a_node     = types an_instance of (types tree node);
      a_function = types a_function;
      an_integer = types an_integer;
      a_max_sum_pair = types a_struct
        named 'max_sum_struct 
        with_fields (|
          sum      = an_integer ;
          max      = an_integer ;
          max_node = a_node ;
        |);
      module has a_function 
        named 'maximal_subtree_and_sum 
        with_input ( a_tree node named 'node )
        returning a_max_sum_struct
        private;
      module has a_function
        named 'maximal_subtree
        with_input ( a_tree named 'tree )
        returning an_integer;
    };

    tests = { test |
      empty_tree  = (| root = nil |);
      empty_node  = (| value = 1 ; children = [] |);
      single_tree = (| root = empty_node |);
      double_tree = (| 
        root = (| 
          value = 1;
          children = [
            (| value = -1 ; children = [] |)
          ]
        |)
      |)
      test { maximal_subtree empty_tree  } expects nil ;
      test { maximal_subtree single_tree } expects empty_node ;
      test { maximal_subtree double_tree } expects double_tree root ;
    };

    impl = (|
      maximal_subtree_and_sum = { node |
        sum      = node value; 
        max      = nil;
        max_node = nil;
        node children 
          select { child  | child != nil                  } 
          map    { child  | maximal_subtree_and_sum child }
          each   { aggregate |
            sum += aggregate sum;
            if ( max == nil || max < aggregate max ) {
              max      = aggregate max;
              max_node = aggregate max_node;
            };
          };
        if ( max == nil || max < sum ) {
          max      = sum;
          max_node = node;
        }
        return (| sum = sum ; max = max ; max_node = max_node |)
      };

      maximal_subtree = { tree |
        root = tree root;
        max_node = nil;
        if ( root != nil ) {
          max_node = maximal_subtree_and_sum root max_node;
        }
        return max_node
      };
    |);
  |)

