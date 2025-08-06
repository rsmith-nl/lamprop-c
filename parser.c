// file: parser.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-04 00:11:34 +0200
// Last modified: 2025-08-06T21:50:05+0200

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
  if (inputfile==0) {
    return contents;
  }
  fseek(inputfile, 0L, SEEK_END);
  ptrdiff_t size = ftell(inputfile);
  rewind(inputfile);
  contents.data = arena_new(permanent, char, size);
  contents.len = size;
  ptrdiff_t rv = fread(contents.data, sizeof(char), size, inputfile);
  fclose(inputfile);
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
  // This function is only called when *line* starts with 'f:'.
  // So discard that.
  // cut.tail now starts with the Young's modulus after whitespace.
  Sv8Double E = sv8tod(cut.tail);
  if (E.ok) {
    rv.E = E.result;
    rv.ok = true;
    //debug("E = %g\n", E.result);
  } else {
    return rv; // empty
  }
  // E.tail now starts with the Poisson constant after whitespace.
  Sv8Double ν = sv8tod(E.tail);
  if (ν.ok) {
    rv.ν = ν.result;
    rv.ok = true;
    //debug("ν = %g\n", ν.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ν.tail now starts with the CTE after whitespace.
  Sv8Double α = sv8tod(ν.tail);
  if (α.ok) {
    rv.α = α.result;
    rv.ok = true;
    //debug("α = %g\n", α.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // α.tail now starts with the density after whitespace.
  Sv8Double ρ = sv8tod(α.tail);
  if (ρ.ok) {
    rv.ρ = ρ.result;
    rv.ok = true;
    //debug("ρ = %g\n", ρ.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ρ.tail now starts with the name after whitespace.
  rv.name = sv8strip(ρ.tail);
  //debug("rv.name.len = %ld\n", rv.name.len);
  rv.ok = true;
  return rv;
}

Fiber parse_fiber(Sv8 line)
{
  Fiber rv = {0};
  Sv8Cut cut = sv8lsplit(line);
  // This function is only called when *line* starts with 'f:'.
  // So discard that.
  // cut.tail now starts with the Young's modulus after whitespace.
  Sv8Double E1 = sv8tod(cut.tail);
  if (E1.ok) {
    rv.E1 = E1.result;
    rv.ok = true;
    //debug("E1 = %g\n", E1.result);
  } else {
    return rv; // empty
  }
  // E1.tail now starts with the Poisson constant after whitespace.
  Sv8Double ν12 = sv8tod(E1.tail);
  if (ν12.ok) {
    rv.ν12 = ν12.result;
    rv.ok = true;
    //debug("ν12 = %g\n", ν12.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ν12.tail now starts with the CTE after whitespace.
  Sv8Double α1 = sv8tod(ν12.tail);
  if (α1.ok) {
    rv.α1 = α1.result;
    rv.ok = true;
    //debug("α1 = %g\n", α1.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // α1.tail now starts with the density after whitespace.
  Sv8Double ρ = sv8tod(α1.tail);
  if (ρ.ok) {
    rv.ρ = ρ.result;
    rv.ok = true;
    //debug("ρ = %g\n", ρ.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ρ.tail now starts with the name after whitespace.
  rv.name = sv8strip(ρ.tail);
  rv.ok = true;
  return rv;
}

