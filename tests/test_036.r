{ sys |

  sys Object extend = { mixin |
    new_object = self new;
    mixin slots each { slot | new_object set key (mixin get slot) };
    return new_object;
  }

  TestArray = Array extend (|
    initialize = { msg |
      sys print msg;
    };
  |)

  t = TestArray new;
  t initialize "Hello World";
};

