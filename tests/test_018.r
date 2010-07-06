{ sys |
  message = "Hello World\n" ;
  print_two = { x y | sys print x ; sys print y } ;
  print_two message message ;
}
