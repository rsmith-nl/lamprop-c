// file: logging.h
// vim:fileencoding=utf-8:ft=cpp:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-04 00:49:24 +0200
// Last modified: 2025-08-08T14:22:13+0200

#pragma once

#undef error
#ifndef NDEBUG
#define error(...)                                            \
  fprintf(stderr, "ERROR %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__);                               \
  fprintf(stderr, "\n");                                      \
  abort()
#else
#define error(...)                \
  fprintf(stderr, "ERROR: ");     \
  fprintf(stderr, __VA_ARGS__);   \
  fprintf(stderr, "\n");          \
  abort()
#endif  // NDEBUG

#undef debug
#ifndef NDEBUG
#define debug(...)                                            \
  fprintf(stderr, "DEBUG %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__);                               \
  fprintf(stderr, "\n")
#else
#define debug(...) (void)0
#endif  // NDEBUG

#undef warn
#ifndef NDEBUG
#define warn(...)                                            \
  fprintf(stderr, "WARNING %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n")
#else
#define warn(...) \
  fprintf(stderr, "WARNING: "); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n")
#endif  // NDEBUG

#undef info
#ifndef NDEBUG
#define info(...)                                            \
  fprintf(stderr, "INFO %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n")
#else
#define info(...) (void)0
#endif  // NDEBUG
