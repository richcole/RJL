{ sys |
  try {
    throw "Hello World\n" ;
    sys print "Failure\n";
  }
  catch { ex |
    sys print ex ;
  }
}
