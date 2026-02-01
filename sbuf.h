// file: buffer.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2025-08-28 23:49:02 +0200
// Last modified: 2026-02-01T02:14:50+0100

// Simple string buffer.
// Mostly conceived for assembling strings.
// Change the definition of SBUF_MAX if you need longer strings.
// For general allocation, use an arena instead!

#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h> // PATH_MAX

#define SBUF_MAX PATH_MAX

typedef struct {
  ptrdiff_t used;
  bool error;
  char data[SBUF_MAX];
} Sbuf;

// Appends at most “len” bytes to “buf” from “str”. 
// Sets “error” to “false” if there is not enough space.
extern void sbuf_append(Sbuf *buf, const char *str, const ptrdiff_t len);
// Appends null-terminated strings “str” to “buf”.
extern inline void sbuf_appends(Sbuf *buf, const char *str);
// Appends using *printf.
extern void sbuf_printf(Sbuf *buf, const char *fmt, ...);
// Returns how much space remains in the buffer “buf”.
extern ptrdiff_t sbuf_remaining(Sbuf *buf);
// Writes the buffer to the designated stream, and flushes the stream.
extern void sbuf_fputs(Sbuf *buf, FILE* stream);
// Empty the buffer.
extern void sbuf_reset(Sbuf *buf);
