#ifndef FOREACH_H
#define FOREACH_H

#define FOR_EACH_ARRAY(cxt, i, var, arr)         \
  Fixnum i = 0;                     \
  Object *var = get_at(cxt, arr, i);                     \
  for(;i < array_length(cxt, arr);++i, (var=get_at(cxt, arr, i)))

#define FOR_EACH_ARRAY_REV(cxt, i, var, arr)     \
  Fixnum i = array_length(cxt, arr)-1;            \
  Object *var = get_at(cxt, arr, i);     \
  for(;i >= 0; --i, (i >= 0 ? (var=get_at(cxt, arr, i)) : 0))

#endif
