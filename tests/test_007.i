:one
  push 1
  pop  1
  push 2
  ret
:two
  push :one
  send
  term
