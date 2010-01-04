{ sys |
  i = 0;
  set_i = { x | i = x };
  set_i 1 ;
  if ( i > 0 ) {
    sys print "Success\n";
  }
  else {
    sys print "Failure\n";
  }
}
