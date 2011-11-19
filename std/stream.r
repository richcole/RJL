{ sys |

  sys Stream: (|
    read_int_lsb: {|
      ( 0 .. 3 ) inject: 0 func: { result index |
        (result << 8) | (self read);
      };
    |};
    read_int_msb: {|
      ( 0 .. 3 ) inject: 0 func: { result index |
        (result | ((self read) << (8 * index)));
      };
    |};
  |);

  sys FileStream: (|
    add_parent: (sys Stream);

    self open:: { filename |
      self new: { 
        self file: (sys File open: filename);
      };
    };

    self delegate: [#read; #eof] to: #file;
  |);
};
