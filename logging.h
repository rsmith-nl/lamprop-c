
#ifndef NDEBUG
#undef debug
#define debug(...)                                            \
  fprintf(stderr, "DEBUG %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__)
#else
#undef debug
#define debug(...) (void)0
#endif  // NDEBUG

#ifndef NDEBUG
#undef error
#define error(...)                                            \
  fprintf(stderr, "ERROR %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__);                               \
  abort()
#else
#undef error
#define error(...) (void)0
#endif  // NDEBUG

#ifndef NDEBUG
#undef warn
#define warn(a, ...)                                              \
  fprintf(stderr, "WARNING %s, line %i: ", __FILE__, __LINE__, ); \
  fprintf(stderr, __VA_ARGS__)
#else
#undef warn
#define warn(...) (void)0
#endif  // NDEBUG

#undef UNUSED
#define UNUSED(x)(void)(x)
