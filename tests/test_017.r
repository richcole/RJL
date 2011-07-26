{ sys |
  try {
    raise: "An exception";
  }
  catch { ex |
    println: (ex reason);
  };
}
