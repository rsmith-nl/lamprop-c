// file: lamprop.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:20:39+0200
// Last modified: 2025-08-03T19:31:01+0200

#include "logging.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>  // for fprintf(3)
#include <stdlib.h> // for abort(3)
#include <string.h>

#undef fatal
#define fatal(...)                                                  \
  fprintf(stderr, "FATAL ERROR %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__);                                     \
  abort()


int main(int argc, char *argv[])
{
  UNUSED(argc);
  UNUSED(argv);
  debug("starting lamprop...\n");
  debug("ending lamprop...\n");
  return 0;
}
