{ sys |

  ByteArray = sys ByteArray;

  sys Object extend = { map |
    new_object = self new;
    map each { key, value | new_object := key value };
    return new_object;
  }

  code_block = ByteArray new size = 10;

  CodeBlock = (|
    initialize = { size |
      self ba = ByteArray new size;
      tail = 0;
    };

    push = { bytes |
      bytes each { byte |
        ba := tail byte;
        tail = tail + 1;
      }
    };
  |)
 
  Instruction = (|
    reg_encoding = (|
      AH = 0; AL = 0; AX = 0; EAX = 0;
      BH = 3; BL = 3; BX = 3; EBX = 3;
      CH = 1; CL = 1; CX = 1; ECX = 1;
      DH = 2; DL = 2; DX = 2; EDX = 2;
      SI = 6; DI = 7; BP = 5; SP  = 4;
    |);
    op_encode = { op_code op1 |
      (op_code << 3) & (reg_encoding : op1)
    };
  |);

  Add = Instruction extend (|
    initialize = { op1 op2 | 
      self op1 = op1;
      self op2 = op2;
    };
    write_to = { cb |
      if ( ( op1 == 'AL ) && ( is_byte op2 ) ) {
        cb push [ op_encode(0x04, op1); (as_byte op2) ];
      }
      elsif ( ( op1 == 'AX ) && ( is_word op2 ) ) {
        cb push [ 0x66; op_encode(0x05, op1); (as_word op2) ];
      }
      elsif ( ( op1 == 'EAX ) && ( is_dword op2 ) ) ) {
        cb push [ op_encode(0x05, op1); (as_dword op2) ];
      }
    };
  |);

}