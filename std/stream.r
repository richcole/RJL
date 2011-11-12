{ sys |

  sys Stream: (|
    read_int_lsb: {|
      result: 0;
      ( 0 .. 3 ) each: { |index|
        result: (result << 8) | (self read);
      };
    |};
    read_int_msb: {|
      result: 0;
      ( 0 .. 3 ) each: { |index|
        result: (result | ((self read) << (8 * ;
      };
    |};
  |);

  sys FileStream: (|

    open:: { filename |
      self new: { 
        self file: (sys File open: filename);
      };
    };

    read: { 
      self file read;
    };

    file: (File open: filename);
  |);
}