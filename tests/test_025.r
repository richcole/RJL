{ sys |
  sys load: "std/std.r";

  Screen init;
  screen: (Screen new);
  screen size: (| self width: 640; self height: 480; |);
  screen fill: 0xff00;
  screen flip;
  sleep: 1;
  Screen close;
};