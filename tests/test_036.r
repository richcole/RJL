{ sys |
  TestArray = Array extend (|
    initialize = { msg |
      sys print msg;
    };
  |)
  TestArray new "Hello World";
};
