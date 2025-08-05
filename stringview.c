// file: sv.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-04-07 22:53:56 +0200
// Last modified: 2025-08-05T22:23:50+0200

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "stringview.h"


static bool _isspace(char c)
{
  if (c==' '||c=='\t'|c=='\r'|c=='\n'||c=='\v'||c=='\f') {
    return true;
  }
  return false;
}

bool sv8equals(Sv8 a, Sv8 b)
{
  return (a.len == b.len) && (!a.len || !memcmp(a.data, b.data, a.len));
}

Sv8 sv8lstrip(Sv8 s)
{
  while (s.len && _isspace(*s.data)) {
    s.data++, s.len--;
  }
  return s;
}

Sv8 sv8rstrip(Sv8 s)
{
  while (s.len && _isspace(s.data[s.len-1])) {
    s.len--;
  }
  return s;
}

Sv8 sv8strip(Sv8 s)
{
  while (s.len && *s.data<=' ') {
    s.data++, s.len--;
  }
  while (s.len && s.data[s.len-1]<=' ') {
    s.len--;
  }
  return s;
}

ptrdiff_t sv8find(Sv8 s, Sv8 f)
{
  if (s.len == 0 || f.len == 0 || f.len > s.len) {
    return -1;
  }
  ptrdiff_t maxoff = s.len - f.len;
  for (ptrdiff_t off = 0; off <= maxoff ; off++) {
    if (memcmp(s.data+off, f.data, f.len) == 0) {
      return off;
    }
  }
  return -1;
}

ptrdiff_t sv8count(Sv8 s, char f)
{
  ptrdiff_t rv = 0;
  for (ptrdiff_t j = 0; j < s.len; j++) {
    if (s.data[j] == f) {
      rv++;
    }
  }
  return rv;
}

Sv8 sv8span(char *beg, char *end)
{
  Sv8 r = {0};
  r.data = beg;
  r.len  = beg ? end-beg : 0;
  return r;
}

Sv8Cut sv8cut(Sv8 s, char c)
{
  Sv8Cut r = {0};
  if (s.len == 0) {
    return r;
  }
  char *beg = s.data;
  char *end = s.data + s.len;
  char *cut = beg;
  for (; cut<end && *cut!=c; cut++) {}
  r.ok   = cut < end;
  r.head = sv8span(beg, cut);
  r.tail = sv8span(cut+r.ok, end);
  return r;
}

Sv8Cut sv8lsplit(Sv8 s)
{
  Sv8Cut r = {0};
  if (s.len == 0) {
    return r;
  }
  char *beg = s.data;
  char *end = s.data + s.len;
  char *cut = beg;
  char *startws = 0, *endws = 0;
  while (cut<end) {
    if (_isspace(*cut)) { // whitespace
      if (startws==0) {
        startws = cut;
      }
    } else { // not whitespace
      if (startws!=0) {
        endws = cut;
        break;
      }
    }
    cut++;
  }
  r.ok   = cut < end;
  r.head = sv8span(beg, startws);
  r.tail = sv8span(endws, end);
  return r;
}

Sv8Int sv8toi(Sv8 s)
{
  Sv8Int rv = {0};
  s = sv8lstrip(s);
  char *beg = s.data;
  char *end = s.data + s.len;
  int32_t state = 0, number = 0;
  bool negative = false;
  bool stop = false;
  while (beg<end && !stop) {
    char c = *beg++;
    switch (state) {
      case 0:
        if (c=='+') {
          state = 1;
        } else if (c=='-') {
          negative = true;
          state = 1;
        } else if (c=='0') {
          state = 2;
        } else if (c>='1' && c <='9') {
          state = 3;
          number = c - '0';
        } else {
          goto fail2;
        }
        break;
      case 1:
        if (c=='0') {
          state = 2;
        } else if (c>='1' && c <='9') {
          state = 3;
          number = c - '0';
        } else {
          goto fail2;
        }
        break;
      case 2:
        if (c=='0') {
          state = 2;
        } else if (c>='1' && c <='9') {
          state = 3;
          number = c - '0';
        } else {
          stop = true;
          beg--;
        }
        break;
      case 3:
        if (c>='0' && c <='9') {
          state = 3;
          number = number*10 + c - '0';
        } else {
          stop = true; // non-numeric character
          if (beg < end) {
            beg--;
          }
        }
        break;
    } // switch
  } // while
  rv.result = number;
  if (negative) {
    rv.result *= -1;
  }
  rv.tail = sv8span(beg, end);
  rv.ok = true;
  return rv;
fail2:
  rv.ok = false;
  rv.result = 0;
  rv.tail = s;
  return rv;
}



Sv8Double sv8tod(Sv8 s)
{
  // See e.g. https://www.codeproject.com/Articles/5291098/\
  // Fun-With-State-Machines-Incrementally-Parsing-Numb
  // See also sv8tod.pdf for the diagram of this state machine.
  Sv8Double rv = {0};
  int32_t state = 0;
  bool stop = false;
  s = sv8lstrip(s);
  char *beg = s.data;
  char *end = s.data + s.len;
  while (beg<end && !stop) {
    char c = *beg++;
    switch (state) {
      case 0:
        if (c=='-') {
          state = 1;
        } else if (c=='0') {
          state = 2;
        } else if (c>='1' && c<='9') {
          state = 8;
        } else {
          goto fail1; // invalid start token
        }
        break;
      case 1:
        if (c=='0') {
          state = 2;
        } else if (c>='1' && c<='9') {
          state = 8;
        } else {
          goto fail1; // non-number after starting '-'.
        }
        break;
      case 2:
        if (c=='.') {
          state = 3;
        } else if (c=='e' || c=='E') {
          state = 5;
        } else {
          goto fail1; // invalid token after starting (-)0.
        }
        break;
      case 3:
        if (c>='0' && c<='9') {
          state = 4;
        } else {
          stop = true;
        }
        break;
      case 4:
        if (c>='0' && c<='9') {
          state = 4;
        } else if (c=='e' || c=='E') {
          state = 5;
        } else {
          stop = true;
        }
        break;
      case 5:
        if (c>='0' && c<='9') {
          state = 7;
        } else if (c=='+' || c=='-') {
          state = 6;
        } else {
          goto fail1; // e or E without number.
        }
        break;
      case 6:
        if (c>='0' && c<='9') {
          state = 7;
        } else {
          goto fail1; // not a number after [eE][+-]
        }
        break;
      case 7:
        if (c>='0' && c<='9') {
          state = 7;
        }
        break;
      case 8:
        if (c=='.') {
          state = 3;
        } else if (c>='0' && c<='9') {
          state = 8;
        } else if (c=='e' || c=='E') {
          state = 5;
        } else {
          stop = true;
        }
        break;
    } // switch
  } // while
  rv.ok = true;
  // TODO: set rv.result and rv.tail.
  return rv;
fail1:
  rv.ok = false;
  rv.result = 0.0;
  rv.tail = s;
  return rv;
}
