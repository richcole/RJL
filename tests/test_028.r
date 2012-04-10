{ sys |

  load: "std/std.r";

  o: (| self name: "test"; |);
  o print: {
    println: (self name);
    catch_throw: {
      println: (self name);
    };
  };
  o print;

};