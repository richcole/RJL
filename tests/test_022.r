{ sys |
  thrower = { msg | throw msg };

  try {
    thrower "Hello World\n" ;
  }
  catch { ex |
    sys print ex ;
  }
}

