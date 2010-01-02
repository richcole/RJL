:one
  push "one"
  ret
:two
  push :one
  push #one=
  send
  pop
  push #one
  send
  term
