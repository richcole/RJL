:one
  push 1
  pop  1
  ret
:two
  push nil
  push nil
  push :one
  call
  term
