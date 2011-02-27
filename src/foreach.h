#define FOR_EACH_ARRAY(i, var, arr) \
  Fixnum i = 0;                     \
  Object *var = get_at(arr, i);     \
  for(;i < array_length(arr);++i, (var=get_at(arr, i)))

