:one
  push "one"
  ret
:two
  push nil
  push nil
  new
  dup
  push #one
  push :one
  assign
  push #one
  send 
  call
  term
