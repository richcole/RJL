{ sys |
  try {
    a: "Test variable";
    raise: "An exception";
  }
  catch { ex |
    println: a;
    println: (ex reason);
  };
}
