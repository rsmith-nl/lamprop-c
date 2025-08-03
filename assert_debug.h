// file: assert_debug.h
// vim:fileencoding=utf-8:ft=c
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// Adapted from: https://nullprogram.com/blog/2022/06/26/
// Created: 2024-09-07T16:11:25+0200
// Last modified: 2024-09-07T16:58:57+0200

#if __clang__
#define assert(c) if (!(c)) __builtin_trap()
#elif __GNUC__
#define assert(c) if (!(c)) __builtin_trap()
#elif _MSC_VER
#define assert(c) if (!(c)) __debugbreak()
#else
#define assert(c) if (!(c)) *(volatile int *)0 = 0
#endif
