.one
  push 1
  jmpnz .two
  push 2
.two
  push 1
  jmpnz .three
  pop
  pop
.three
  term
