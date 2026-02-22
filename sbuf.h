// file: sbuf.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2025-08-28 23:49:02 +0200
// Last modified: 2026-02-22T11:28:46+0100

// Simple string buffer.
// Mostly conceived for assembling strings.
// Change the definition of SBUF_SIZE if you need longer strings.
// For general allocation, use an arena instead!

#pragma once

#include <stddef.h>  // for ptrdiff_t
#include <stdbool.h> // for bool
#include <stdio.h>   // for FILE*
#include <limits.h>  // for PATH_MAX

#define SBUF_SIZE PATH_MAX

typedef struct {
  ptrdiff_t used;
  bool error;  // use “error” instead of “ok” so a zerod-out Sbuf is valid.
  char data[SBUF_SIZE];
} Sbuf;

#ifdef __cplusplus
extern "C" {
#endif

// All appends set “error” to “true” if there is not enough space.
// All appends immediately return if “error” is “true”.

// Appends at most “len” bytes to “buf” from “str”.
extern void sbuf_append(Sbuf *buf, const char *str, const ptrdiff_t len);

// Appends null-terminated strings “str” to “buf”.
extern void sbuf_appends(Sbuf *buf, const char *str);

// Appends using *printf.
extern void sbuf_printf(Sbuf *buf, const char *fmt, ...);

// Returns how much space remains in the buffer “buf”.
extern ptrdiff_t sbuf_remaining(Sbuf *buf);

// Writes the buffer to the designated stream, and flushes the stream.
extern void sbuf_fputs(Sbuf *buf, FILE* stream);

// Empty the buffer.
extern void sbuf_reset(Sbuf *buf);

#ifdef __cplusplus
}
#endif
