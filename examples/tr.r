= tr 

This program is an example of literate programming in RJL. Any
non-indented line is considered to be documentation and follows a
simple wiki style. Any indented line is part of the code.

This module defines a single function named tr. The module has a
section for defining types, tests and dependencies.

You notices that all statements in RJL are executable code including
type annotations. In RJL these things, types, tests and dependencies
are all provided by libraries.

Lets begin with the outer definition. It is a block map. That is a
block that when executed returns an object with fields defined by
assignments. The block takes sys as an argument which provides access
to the runtime.

  [ sys |

The next section defines the types of the module being returned.

    types = { types, module |
      a_string   = types a_string;
      a_function = types a_function;
      module has (
        a_function named 'tr
          with_input [|
            a_string named "from_set" ; 
            a_string named "to_set"   ; 
            a_string named "input"    ;
          |] 
          returning string
      );
    };

This block will be executed both with an argument type which is used
to define types and within a lexical scope which provides access to
existing types.

[Note] Previously I had this function being executed within a lexical
scope. That lexical scope gave definition to the free variables. I
have echewed this notion however in favor of passing the variables
in. [End]

Next we define a couple of tests for the module.

    tests = { test |
      test { tr "ab" "bc" "abc" } expects "bcc" ;
      test { tr "ab" "b" "abc"  } throws ArgumentError ;
    };

Next we define the dependencies. This block is passed an object called
'it' which is used to declare dependencies.

    depends = { module |
      a_module        = module a_module;
      module name     = "org.richcole.examples.tr";
      module version  = "0.1";
      module requires [| 
        a_module named "std.types"; 
        a_module named "std.map"; 
        a_module named "std.errors";
      |];
    ];

The module being defined is called "org.richcole.example.tr". This
module requires three modules to run. These are std.types,
std.map and std.errors.

Next comes the implementation for the module. In a more general
setting one might have several implementations for a single type
definition, however in this case we have the implementation and type
definitions all defined in one place.

We are not commenting at this stage on how a given runtime system will
locate and install the dependencies neccessary to run a module.

    implementation = [ module |
      module ArgumentError = module std error ArgumentError ;
      module tr = { 
        from_set : string ; 
        to_set   : string ; 
        input    : string ;
      |
        if (from_set length) != (to_set length) {
          throw module ArgumentError new 
            "Expected from_set and to_set to have the same length";
        }
        return input map { ch | 
          index = from_set index ch ;
          if ( index ) {
            return to_set at index;
          }
          else {
            return ch;
          }
        }
      }
    ]

  ]

