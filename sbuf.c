// file: buffer.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2025-08-28 23:49:02 +0200
// Last modified: 2026-02-22T11:28:54+0100

#include "sbuf.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

void sbuf_append(Sbuf *buf, const char *str, const ptrdiff_t len)
{
  assert(buf!=0);
  assert(str!=0);
  if (buf->error == true) {
    return;
  }
  ptrdiff_t alen = strnlen(str, len);
  ptrdiff_t remaining = SBUF_SIZE - buf->used - 1;
  if (len < remaining) {
    memcpy(buf->data+buf->used, str, alen);
    buf->used += alen;
    buf->error = false;
  } else {
    buf->error = true;
  }
}

inline void sbuf_appends(Sbuf *buf, const char *str)
{
  assert(buf!=0);
  if (buf->error == true) {
    return;
  }
  ptrdiff_t remaining = SBUF_SIZE - buf->used - 1;
  sbuf_append(buf, str, strnlen(str, remaining));
}

void sbuf_printf(Sbuf *buf, const char *fmt, ...)
{
  assert(buf!=0);
  assert(fmt!=0);
  if (buf->error == true) {
    return;
  }
  ptrdiff_t remaining = SBUF_SIZE - buf->used - 1;
  va_list ap;
  va_start(ap, fmt);
  ptrdiff_t used = vsnprintf(buf->data+buf->used, remaining, fmt, ap);
  va_end(ap);
  if (used > remaining) { // discard
    memset(buf->data+buf->used, 0, remaining);
    buf->error = true;
  } else {
    buf->error = false;
    buf->used += used;
  }
}

ptrdiff_t sbuf_remaining(Sbuf *buf)
{
  assert(buf!=0);
  ptrdiff_t remaining = SBUF_SIZE - buf->used - 1;
  return remaining;
}

void sbuf_fputs(Sbuf *buf, FILE* stream)
{
  assert(buf!=0);
  assert(stream!=0);
  fputs(buf->data, stream);
  fflush(stream);
}

void sbuf_reset(Sbuf *buf)
{
  assert(buf!=0);
  memset(buf->data, 0, SBUF_SIZE);
  buf->used = 0;
  buf->error = false;
}
