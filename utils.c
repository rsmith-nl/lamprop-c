// file: utils.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2026 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2026-02-01 16:14:19 +0100
// Last modified: 2026-02-04T20:23:52+0100

#include <assert.h>
#include <math.h>

double frexp10(double arg, int *exp)
{
  assert(exp!=0);
  *exp = (arg == 0) ? 0 : (int)floor(log10(fabs(arg)));
  return arg * pow(10, -(*exp));
}




