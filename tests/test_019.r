{ sys |
  print_fn = { x | sys print x } ;
  call = { fn x | fn x } ;
  call print_fn "Hello World\n" ; 
}
