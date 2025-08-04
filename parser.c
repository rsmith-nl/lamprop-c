// file: parser.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-04 00:11:34 +0200
// Last modified: 2025-08-04T23:48:54+0200

#include "arena.h"
#include "stringview.h"
#include "logging.h"
#include "types.h"

//#include <stdint.h>
//#include <stdbool.h>
#include <stdio.h>  // for fopen
//#include <stdlib.h>
#include <sys/mman.h> // for mmap

Sv8 read_file(char *path, Arena *permanent)
{
  Sv8 contents = {0};
  FILE *inputfile = fopen(path, "r");
  fseek(inputfile, 0L, SEEK_END);
  ptrdiff_t size = ftell(inputfile);
  rewind(inputfile);
  contents.data = arena_new(permanent, char, size);
  contents.len = size;
  ptrdiff_t rv = fread(contents.data, sizeof(char), size, inputfile);
  if (rv != size) {
    fprintf(stderr,
            "WARNING: file “%s” has size %td bytes, but only %td bytes read.\n",
            path, size, rv);
  }
  return contents;
}

Resin parse_resin(Sv8 line)
{
  Resin rv = {0};
  Sv8Cut cut = sv8lsplit(line);
  // This function is only called when *line* starts with 'r:'.
  // So discard that.
  cut = sv8lsplit(cut.tail);
  //cut.head contains the Young's modulus.

  return rv;
}

Fiber parse_fiber(Sv8 line)
{
  // This function is only called when *line* starts with 'f:'.
  // So we don't have to check that.
  Fiber rv = {0};
  return rv;
}

