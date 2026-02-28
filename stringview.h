// file: stringview.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2025-04-07 22:53:56 +0200
// Last modified: 2026-02-27T20:37:14+0100

// Inspired by: https://nullprogram.com/blog/2023/10/08/

#pragma once
#include <stddef.h>     // ptrdiff_t
#include <stdbool.h>    // bool
#include <stdint.h>     // int32_t

// The following works with string constants.
// Note that it does not work with members of an array of char pointers!
#define SV8(s)    (Sv8){s, sizeof(s)-1}

// Note that these structures do not allocate memory.
// So they are *views* of a existing string in memory somewhere.

// String view
typedef struct {
  char *data;
  ptrdiff_t len;
} Sv8;

typedef struct {
  Sv8 head;
  Sv8 tail;
  bool ok;
} Sv8Cut;

typedef struct {
  double result;
  Sv8 tail;
  bool ok;
} Sv8Double;

typedef struct {
  int32_t result;
  Sv8 tail;
  bool ok;
} Sv8Int;

#ifdef __cplusplus
extern "C" {
#endif

// Test two strings for equality of length and content.
extern bool sv8equals(Sv8 a, Sv8 b);

// Return true if s starts with t.
extern bool sv8startswith(Sv8 s, Sv8 t);

// Return true if s ends with e.
extern bool sv8endswith(Sv8 s, Sv8 e);

// Return true if s contains with c.
extern bool sv8contains(Sv8 s, Sv8 c);

// Remove whitespace from the left of a string.
extern Sv8 sv8lstrip(Sv8 s);

// Strip num characters from the left of the string.
extern Sv8 sv8lskip(Sv8 s, int32_t num);

// Strip num characters from the right of the string.
extern Sv8 sv8rskip(Sv8 s, int32_t num);

// Remove whitespace from the right of a string.
extern Sv8 sv8rstrip(Sv8 s);

// Remove whitespace from both ends of the string.
extern Sv8 sv8strip(Sv8 s);

// Return the offset of the string f in the string s.
// If not found, return -1.
extern ptrdiff_t sv8find(Sv8 s, Sv8 f);

// Return the amount of occurrances of the character f in string s.
extern ptrdiff_t sv8count(Sv8 s, char f);

// Return the Sv8 that stretched from beg to end.
extern Sv8 sv8span(char *beg, char *end);

// Cut the string s at the character c.
extern Sv8Cut sv8cut(Sv8 s, char c);

// Cut the string at the first whitespace from the left.
extern Sv8Cut sv8lsplit(Sv8 s);

// Tries to read an int32_t from the beginning of a stringview.
// Leading whitespace is skipped.
// If an integer was found, Sv8Int.ok equals true, otherwise false.
// A view of the remaining string is returned in Sv8Int.tail.
extern Sv8Int sv8toi(Sv8 s);

// Tries to read a double from the beginning of a stringview.
// Leading whitespace is skipped.
// If an floating point number was found, Sv8Double.ok equals true, otherwise false.
// A view of the remaining string is returned in Sv8Double.tail.
// Does not handle NaN and Inf!
Sv8Double sv8tod(Sv8 s);

// Hash a string view with FNV-1a hash function.
extern uint64_t sv8hash64(Sv8 s);

// Return up to 255 bytes of the Sv8 as a C string.
// This returns a pointer to a static buffer.
extern char *sv8cstring(Sv8 s);

#ifdef __cplusplus
}
#endif
