{ sys |

  sys load: "std/std.r";

  sys slots each: { slot |
    if ( slot == "Array" ) {
      println: slot;
    };
  };
};