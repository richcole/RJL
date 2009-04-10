.one
  push 1
  jmp .two
  push 2
.two
  push 1
  jmp .three
  pop
  pop
.three
  term
