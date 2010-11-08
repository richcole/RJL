{ sys |

  Utf8Stream = [|
    initialize = { stream |
      self stream = stream ;
      self invalid_stream = false;
    };

    is_utf8_lob = { ch |
      return (ch & 0xc0) == 0x80;
    };

    throw_invalid_unicode_character = {
      self invalid_stream = true;
      e = InvalidUnicodeCharacter new;
      e byte_offset = self byte_offset;
      e stream = self;
      throw e;
    };

    byte_offset = {
      stream byte_offset;
    };

    next = {
      if (self eof) {
        throw EndOfFileException new;
      }
      ch = self stream next ;
      ch3 = ch2 = ch1 = nil;
      if ( ch & 0x80 ) {
        return ch;
      }
      else if ( (ch & 0x80) && (ch & 0x40) ) {
        if ( ch & 0x20 ) {
          if ( ch & 0x10 ) {
            invalid_eof {
              ch1 = self stream next;
              ch2 = self stream next;
              ch3 = self stream next;
            }
            if ( is_utf8_lob(ch1) && is_utf8_lob(ch2) && is_utf8_lob(ch3) ) {
              return (ch3 & 0x7) | (ch2 << 6) | (ch1 << 12) | (ch << 18);
            }
            else {
              throw_invalid_unicode_character;
            }
          }
          else {
            invalid_eof {
              ch1 = self stream next;
              ch2 = self stream next;
            }
            if ( is_utf8_lob(ch1) && is_utf8_lob(ch2) ) {
              return (ch2 & 0xf) | (ch1 << 6) | (ch << 12);
            }
            else {
              throw_invalid_unicode_character;
            }
          }
        }
        else {
          invalid_eof {
            ch1 = self stream next;
          }
          if ( is_utf8_lob(ch1) ) {
            return ((ch & 1f) << 6) | (ch1 & 3f);
          }
          else {
            throw invalid_unicode_character;
          }
        }
      }
      else {
        throw_invalid_unicode_character;
      }
    };

    private invalid_eof = { block |
      try {
        block;
      }
      catch { e |
        if ( e is_a EndOfFileException ) {
          throw_invalid_unicode_character;
        }
      }
    };

    eof = {
      return (self invalid_stream) || (self stream eof) ;
    };
  |];

  StringStream = [|
    initialize = { string |
      self string = string;
      self index = 0;
    };

    next = {
      if ( ! (self eof) ) {
        ch = string byte_at index;
        self index += 1;
        return ch;
      }
      else {
        throw EndOfFileException new;
      }
    }

    eof = {
      return index >= (string length);
    }
  |];

  LineCountingStream = [|
    initialize = { stream |
      self stream = stream;
      self line_number = 0;
      self char_number = 0;
      self prev_ch = nil;
    };

    next = {
      ch = self stream next;
      
      if ( ch == 0xd ) { 
        self line_number += 1;
      }
      else if ( ch == 0xa ) {
        if ( prev_ch == 0xd ) {
          prev_ch = ch;
          return self next;
        }
        self line_number += 1;
      }
      
      prev_ch = ch;
      return ch;
    }  

    eof = {
      return self stream eof;
    }
  }

  path = [];
  try {
    while ( true ) {
      line = readline;
      while ( path peek has_greater_indent line ) {
        path pop;
      }
      if ( line begins_section ) {
        if ( line has_greater_indent (path peek) ) {
          path push (path peek new_child line);
        }
        else if ( line has_equal_indent (path peek) ) {
          path pop;
          path push (path peek new_child line);
        }
      }
      else {
        if ( line has_equal_ident (path peek) ) {
          path peek append_line line
        }
        else if ( line has_greater_indent
    }
  }
  catch { e |
    if ( e is_a EndOfFileException ) {
    }
    else {
      throw e;
    }
  }
  

  
}