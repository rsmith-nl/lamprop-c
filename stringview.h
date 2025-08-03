// file: sv.h
// vim:fileencoding=utf-8:ft=cpp:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-04-07 22:53:56 +0200
// Last modified: 2025-08-03T21:44:24+0200

#pragma once
#include <stddef.h>  // ptrdiff_t
#include <stdbool.h>  // bool

#define SV8(s)    (Sv8){s, sizeof(s)-1}

// Note that these structures do not allocate memory.
// So they are *views* of a existing string in memory somewhere.

typedef struct {
  char *data;
  ptrdiff_t len;
} Sv8;

typedef struct {
  Sv8 head;
  Sv8 tail;
  bool ok;
} Sv8Cut;


#ifdef __cplusplus
extern "C" {
#endif

// Test two strings for equality of length and content.
extern bool sv8equals(Sv8 a, Sv8 b);

// Remove whitespace from the left of a string.
extern Sv8 sv8lstrip(Sv8 s);

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


#ifdef __cplusplus
}
#endif
