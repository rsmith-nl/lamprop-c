// file: logging.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-04 00:49:24 +0200
// Last modified: 2026-02-17T21:39:23+0100

#pragma once

#include <stdio.h>
#include <stdlib.h>  // for abort

#define NM "#lamprop-c "

#undef error
#ifndef NDEBUG
#define error(...) \
  fprintf(stderr, NM"ERROR %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  fflush(stderr); \
  abort()
#else
#define error(...) \
  fprintf(stderr, NM"ERROR: "); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  fflush(stderr); \
  abort()
#endif  // NDEBUG

#undef debug
#ifndef NDEBUG
#define debug(...) \
  fprintf(stderr, NM"DEBUG %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  fflush(stderr)
#else
#define debug(...) (void)0
#endif  // NDEBUG

#undef warn
#ifndef NDEBUG
#define warn(...)                                            \
  fprintf(stderr, NM"WARNING %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  fflush(stderr)
#else
#define warn(...) \
  fprintf(stderr, NM"WARNING: "); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  fflush(stderr)
#endif  // NDEBUG

#undef info
#ifndef NDEBUG
#define info(...) \
  fprintf(stderr, NM"INFO %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  fflush(stderr)
#else
#define info(...) \
  fprintf(stderr, NM"INFO: "); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  fflush(stderr)
#endif  // NDEBUG
