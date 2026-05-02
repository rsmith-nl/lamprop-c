// file: sbuf.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2025-08-28 23:49:02 +0200
// Last modified: 2026-04-23T18:53:20+0200

#include "sbuf.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void sbuf_append(Sbuf *buf, char *str, ptrdiff_t len)
{
  assert(buf != 0);
  assert(str != 0);
  if (buf->error == true) {
    return;
  }
  ptrdiff_t alen = strnlen(str, len);
  ptrdiff_t remaining = SBUF_SIZE - buf->used - 1;
  if (len < remaining) {
    memcpy(buf->data + buf->used, str, alen);
    buf->used += alen;
    buf->error = false;
  } else {
    buf->error = true;
  }
}

void sbuf_appendc(Sbuf *buf, char c)
{
  assert(buf != 0);
  if (buf->error == true) {
    return;
  }
  ptrdiff_t remaining = SBUF_SIZE - buf->used - 1;
  if (remaining >= 1) {
    buf->data[buf->used++] = c;
    buf->error = false;
  } else {
    buf->error = true;
  }
}

inline void sbuf_appends(Sbuf *buf, char *str)
{
  sbuf_append(buf, str, strlen(str));
}

extern void sbuf_appendi(Sbuf *buf, int64_t i)
{
  assert(buf != 0);
  if (buf->error == true) {
    return;
  }
#define BUFLENI 21
#define ORD0 48
  int64_t work = 0;
  bool negative = false;
  if (i < 0) {
    work = -i;
    negative = true;
  } else {
    work = i;
  }
  char tmpbuf[BUFLENI] = {0};
  int index = sizeof(tmpbuf) - 1;
  do {
    ldiv_t result = ldiv(work, 10);
    tmpbuf[index--] = ORD0 + result.rem;
    work = result.quot;
  } while (work > 0 && index > 1);
  if (negative) {
    assert(index > 0);
    tmpbuf[index--] = '-';
  }
  sbuf_appends(buf, tmpbuf + index + 1);
}

static double frexp10(double arg, int *exp)
{
  *exp = (arg == 0) ? 0 : (int)floor(log10(fabs(arg)));
  return arg * pow(10, -(*exp));
}

extern void sbuf_appendd(Sbuf *buf, double f)
{
#define BUFLEND 30
#define EXPLEN (BUFLEND-4-1)
  assert(buf != 0);
  if (buf->error != false) {
    return;
  }
  int exp;
  double mantissa = frexp10(f, &exp);
  double am = fabs(mantissa);
  char tbuf[BUFLEND] = {0};
  int bufused = 0;
  if (signbit(f)) {
    tbuf[bufused++] = '-';
  }
  if (isinf(f)) {
    char inf[4] = {0xE2, 0x88, 0x9E, 0}; // UTF-8 for infinity symbol.
    sbuf_appends(buf, inf);
    return;
  } else if (mantissa == 0 && exp == 0) {
    tbuf[bufused++] = '0';
    sbuf_appends(buf, tbuf);
    return;
  } else if (isnan(f)) {
    sbuf_appends(buf, "NaN");
    return;
  }
  if (exp > 0 && exp < 10) {
    // Format as dddd.dddddd, with 6 decimal places.
    // Format integer part
    int value = (int)f;
    int frac = (int)round((f - (double)value) * 1e6);
    char intbuf[11] = {0};
    int index = sizeof(intbuf) - 2;
    do {
      div_t result = div(value, 10);
      intbuf[index--] = ORD0 + result.rem;
      value = result.quot;
    } while (value > 0 && index > 0);
    strcpy(tbuf + bufused, intbuf + index + 1);
    bufused += strlen(intbuf + index + 1);
    if (frac == 0) {
      sbuf_appends(buf, tbuf);
      return;
    }
    if (buf->decsep == 0) {
      tbuf[bufused++] = '.';
    } else {
      tbuf[bufused++] = buf->decsep;
    }
    char fracbuf[8] = {0};
    index = sizeof(fracbuf) - 2;
    do {
      div_t result = div(frac, 10);
      fracbuf[index--] = ORD0 + result.rem;
      frac = result.quot;
    } while (frac > 0 && index > 0);
    // Remove trailing 0 decimals
    int ridx = sizeof(fracbuf) - 3;
    while (fracbuf[ridx] == '0') {
      fracbuf[ridx--] = 0;
    }
    strcpy(tbuf + bufused, fracbuf + index + 1);
    sbuf_appends(buf, tbuf);
    return;
  }
  int decimal = (int)floor(am);
  tbuf[bufused++] = ORD0 + decimal;
  am = (am - decimal) * 10.0;
  if (buf->decsep != 0) {
    tbuf[bufused++] = buf->decsep;
  } else {
    tbuf[bufused++] = '.';
  }
  int loopexp = 0;
  do {
    decimal = (int)floor(am);
    tbuf[bufused++] = ORD0 + decimal;
    am = (am - decimal) * 10.0;
    frexp10(am, &loopexp);
    if (decimal == 9 && am > 9.5) {
      break;
    }
  } while (am > 0 && loopexp == 0 && bufused < EXPLEN);
  tbuf[bufused++] = 'e';
  if (exp < 0) {
    tbuf[bufused++] = '-';
    exp = -exp;
  } else {
    tbuf[bufused++] = '+';
  }
  if (exp > 100) {
    tbuf[bufused++] = ORD0 + 1;
    exp -= 100;
  }
  if (exp > 10) {
    tbuf[bufused++] = ORD0 + exp / 10;
    exp -= 10;
  }
  tbuf[bufused++] = ORD0 + exp;
  sbuf_appends(buf, tbuf);
}

void sbuf_printf(Sbuf *buf, char *fmt, ...)
{
  assert(buf != 0);
  assert(fmt != 0);
  if (buf->error == true) {
    return;
  }
  ptrdiff_t remaining = SBUF_SIZE - buf->used - 1;
  va_list ap;
  va_start(ap, fmt);
  ptrdiff_t used = vsnprintf(buf->data + buf->used, remaining, fmt, ap);
  va_end(ap);
  if (used > remaining) { // discard
    memset(buf->data + buf->used, 0, remaining);
    buf->error = true;
  } else {
    buf->error = false;
    buf->used += used;
  }
}

ptrdiff_t sbuf_remaining(Sbuf *buf)
{
  assert(buf != 0);
  ptrdiff_t remaining = SBUF_SIZE - buf->used - 1;
  return remaining;
}

void sbuf_fputs(Sbuf *buf, FILE* stream)
{
  assert(buf != 0);
  assert(stream != 0);
  fputs(buf->data, stream);
  fflush(stream);
}

void sbuf_reset(Sbuf *buf)
{
  assert(buf != 0);
  memset(buf->data, 0, SBUF_SIZE);
  buf->used = 0;
  buf->error = false;
}
