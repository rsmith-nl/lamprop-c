
// For condition testing, e.g. a == b.
#define test(exp) \
  do \
    if (exp) { \
      fprintf(stderr, "\033[0;32mPASSED:\033[0m " #exp "\n"); \
    } else { \
      fprintf(stderr, "\033[1;31mFAILED:\033[0m " #exp "\n"); \
    } \
  while (0)

// Exact equality is too restrictive with floating point.
#define EPS 1e-3
#define isf(var, value) \
  do \
    if ( (var) > ((value)-EPS) && (var) < ((value)+EPS)) { \
      fprintf(stderr, "\033[0;32mPASSED:\033[0m " #var " == " #value "\n"); \
    } else { \
      fprintf(stderr, "\033[1;31mFAILED:\033[0m " #var " != " #value "\n"); \
    } \
  while (0)
