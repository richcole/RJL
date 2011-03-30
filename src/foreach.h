#define FOR_EACH_ARRAY(i, var, arr) \
  Fixnum i = 0;                     \
  Object *var = get_at(arr, i);     \
  for(;i < array_length(arr);++i, (var=get_at(arr, i)))

#define FOR_EACH_ARRAY_REV(i, var, arr) \
  Fixnum i = array_length(arr)-1;   \
  Object *var = get_at(arr, i);     \
  for(;i >= 0; --i, (i > 0 ? (var=get_at(arr, i)) : 0))

