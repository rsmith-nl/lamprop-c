// file: logging.h
// vim:fileencoding=utf-8:ft=cpp:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-04 00:49:24 +0200
// Last modified: 2025-08-04T20:10:19+0200

#pragma once

#ifndef NDEBUG
#undef debug
#define debug(...)                                            \
  fprintf(stderr, "DEBUG %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__)
#else
#undef debug
#define debug(...) (void)0
#endif  // NDEBUG

#undef error
#define error(...)                                            \
  fprintf(stderr, "ERROR %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__);                               \
  abort()

#ifndef NDEBUG
#undef warn
#define warn(...)                                            \
  fprintf(stderr, "WARNING %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n")
#else
#undef warn
#define warn(...) (void)0
#endif  // NDEBUG


#ifndef NDEBUG
#undef info
#define info(...)                                            \
  fprintf(stderr, "INFO %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n")
#else
#undef info
#define info(...) (void)0
#endif  // NDEBUG

#undef UNUSED
#define UNUSED(x)(void)(x)
