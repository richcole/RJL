{ sys |
  set_i = { x | i = x };
  set_i 1 ;
  if ( i ) {
    sys print "Failure\n";
  }
  else { 
    sys print "Success\n";
  }
}
