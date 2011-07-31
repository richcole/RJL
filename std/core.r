{ sys |

  sys load:: { filename |
    file: (File open: filename);
    sc: (Scanner tokenize: file);
    pc: (Parser parse: sc);
    if ( pc errors length == 0 ) {
      context: (CodeGenerator generate: pc);
      context code: sys;
    }
    else {
      println: "Error parsing file";
      dump: (pc errors) to: 6;
    };
  };

  sys load: (args pop);
};
  