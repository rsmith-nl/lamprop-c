// file: sbuf.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2025-08-28 23:49:02 +0200
// Last modified: 2026-10-24T13:52:06+0200

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
  assert(len >= 0);
  if (buf->error == true) {
    return;
  }
  ptrdiff_t alen = strnlen(str, len);
  ptrdiff_t remaining = SBUF_MAX - buf->used;
  if (len < remaining) {
    memcpy(buf->data + buf->used, str, alen);
    buf->used += alen;
    buf->error = false;
    buf->data[buf->used] = 0;
    buf->data[SBUF_MAX] = 0;
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
  ptrdiff_t remaining = SBUF_MAX - buf->used;
  if (remaining >= 1) {
    buf->data[buf->used++] = c;
    buf->error = false;
    buf->data[buf->used] = 0;
    buf->data[SBUF_MAX] = 0;
  } else {
    buf->error = true;
  }
}

inline void sbuf_appends(Sbuf *buf, char *str)
{
  sbuf_append(buf, str, strnlen(str, SBUF_MAX));
}

extern void sbuf_appendi(Sbuf *buf, int64_t i)
{
  assert(buf != 0);
  if (buf->error == true) {
    return;
  }
#define BUFLENI 32
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
  index++;
  sbuf_append(buf, tmpbuf + index, strnlen(tmpbuf + index, BUFLENI - index));
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
    sbuf_append(buf, inf, 4);
    return;
  } else if (mantissa == 0 && exp == 0) {
    tbuf[bufused++] = '0';
    sbuf_append(buf, tbuf, BUFLEND);
    return;
  } else if (isnan(f)) {
    sbuf_append(buf, "NaN", 3);
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
    strncpy(tbuf + bufused, intbuf + index + 1, BUFLEND - bufused - 1);
    bufused += strlen(intbuf + index + 1);
    if (frac == 0) {
      sbuf_append(buf, tbuf, BUFLEND);
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
    strncpy(tbuf + bufused, fracbuf + index + 1, BUFLEND - bufused - 1);
    sbuf_append(buf, tbuf, BUFLEND);
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
  sbuf_append(buf, tbuf, BUFLEND);
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
    buf->data[buf->used] = 0;
    buf->data[SBUF_MAX] = 0;
  }
}

ptrdiff_t sbuf_remaining(Sbuf *buf)
{
  assert(buf != 0);
  ptrdiff_t remaining = SBUF_MAX - buf->used;
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

ptrdiff_t sbuf_find(Sbuf *buf, char *f)
{
  assert(buf != 0);
  if (buf->error == true) {
    return -1;
  }
  ptrdiff_t fl = strnlen(f, SBUF_MAX);
  if (buf->used == 0 || fl == 0 || fl > buf->used) {
    return -1;
  }
  for (ptrdiff_t j = 0; j < buf->used - fl; j++) {
    if (memcmp(buf->data + j, f, fl) == 0) {
      return j;
    }
  }
  return -1;
}

void sbuf_replace_char(Sbuf *buf, char from, char to)
{
  assert(buf != 0);
  if (buf->error == true) {
    return;
  }
  for (ptrdiff_t j = 0; j < buf->used; j++) {
    if (buf->data[j] == from) {
      buf->data[j] = to;
    }
  }
  buf->data[buf->used] = 0;
  buf->data[SBUF_MAX] = 0;
}

void sbuf_replace_string(Sbuf *buf, char *from, char *to)
{
  assert(buf != 0);
  if (buf->error == true) {
    return;
  }
  ptrdiff_t fromlen = strnlen(from, SBUF_MAX);
  ptrdiff_t tolen = strnlen(to, SBUF_MAX);
  ptrdiff_t offset = tolen - fromlen;
  if (fromlen > buf->used) {
    return;
  }
  for (ptrdiff_t j = 0; j <= buf->used - fromlen; j++) {
    if (buf->data[j] == from[0] && strncmp(buf->data + j, from, fromlen) == 0) {
      // Found “from”.
      // Make space if “to” is longer than “from”.
      if (tolen > fromlen) {
        // Return if not enough room is available.
        if (sbuf_remaining(buf) < (tolen - fromlen)) {
          buf->error = true;
          return;
        }
        for (ptrdiff_t k = buf->used + offset; k > j + fromlen; k--) {
          buf->data[k] = buf->data[k - 1];
        }
        buf->used += offset;
      }
      // Replace
      strncpy(buf->data + j, to, tolen);
      // Move remainder if “to” is shorter than “from”.
      if (fromlen > tolen) {
        for (ptrdiff_t k = j + tolen; k < buf->used; k++) {
          buf->data[k] = buf->data[k - offset];
        };
      }
      // Move to past replacement.
      j += tolen;
    }
  }
  buf->data[buf->used] = 0;
  buf->data[SBUF_MAX] = 0;
}


void sbuf_ascii_lower(Sbuf *buf)
{
  assert(buf != 0);
  if (buf->error == true) {
    return;
  }
  for (ptrdiff_t j = 0; j < buf->used; j++) {
    if (buf->data[j] > 64 && buf->data[j] < 91) {
      buf->data[j] += 32;
    }
  }
  buf->data[buf->used] = 0;
  buf->data[SBUF_MAX] = 0;
}
