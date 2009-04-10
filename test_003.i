:one
  push 1
  pop  1
  ret
:two
  push :one
  call
  term
