// file: buffer.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2025-08-28 23:49:02 +0200
// Last modified: 2026-02-01T01:35:25+0100

#include "sbuf.h"
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

void sbuf_append(Sbuf *buf, const char *str, const ptrdiff_t len)
{
  if (buf->error == true) {
    return;
  }
  ptrdiff_t alen = strnlen(str, len);
  ptrdiff_t remaining = SBUF_MAX - buf->used - 1;
  if (len < remaining) {
    memcpy(buf->data+buf->used, str, alen);
    buf->used += alen;
    buf->error = false;
  } else {
    buf->error = true;
  }
}

void sbuf_printf(Sbuf *buf, const char *fmt, ...)
{
  if (buf->error == true) {
    return;
  }
  ptrdiff_t remaining = SBUF_MAX - buf->used - 1;
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
  ptrdiff_t remaining = SBUF_MAX - buf->used - 1;
  return remaining;
}

void sbuf_fputs(Sbuf *buf, FILE* stream)
{
  fputs(buf->data, stream);
  fflush(stream);
}

void sbuf_reset(Sbuf *buf)
{
  memset(buf->data, 0, SBUF_MAX);
  buf->used = 0;
  buf->error = false;
}
