// file: parser.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-04 00:11:34 +0200
// Last modified: 2025-08-08T20:29:58+0200

#include "arena.h"
#include "stringview.h"
#include "logging.h"
#include "types.h"

#include <stdio.h>  // for fopen
#include <string.h> // for memset(3), memcpy(3)

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
  // This function is only called when *line* starts with 'f:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
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
  // This function is only called when *line* starts with 'f:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
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

Laminate parse_laminate(Sv8 line)
{
  Laminate rv = {0};
  rv.ok = true;
  rv.finished = false;
  // This function is only called when *line* starts with 't:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
  // cut.tail now starts with the name after whitespace.
  rv.name = sv8strip(cut.tail);
  if (rv.name.len==0) {
    warn("laminate without a name found");
    rv.ok = false;
  }
  return rv;
}

Mline parse_m(Sv8 line)
{
  Mline rv = {0};
  // This function is only called when *line* starts with 'm:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
  // cut.tail now starts with the fiber volume fraction after whitespace.
  Sv8Double vf = sv8tod(cut.tail);
  if (vf.ok) {
    rv.vf = vf.result;
  } else {
    return rv;
  }
  // vf.tail should now contain the name of the resin.
  Sv8 resin_name = sv8strip(vf.tail);
  if (resin_name.len != 0) {
    rv.resin_name = resin_name;
    rv.ok = true;
  }
  return rv;
}

FRdata fibers_and_resins(Sv8 contents, bool info)
{
  FRdata rv = {0};
  rv.resina = arena_create(NRESINS*sizeof(Resin));
  rv.resins = (void*)rv.resina.begin;
  rv.fibera = arena_create(NRESINS*sizeof(Fiber));
  rv.fibers = (void*)rv.fibera.begin;
  int32_t lineno = 1;
  Sv8Cut ccut = sv8cut(contents, '\n');
  Fiber f = {0};
  Resin r = {0};
  while (ccut.ok == true) {
    if (ccut.head.data[1] == ':') {
      switch (ccut.head.data[0]) {
        case 'f':
          f = parse_fiber(ccut.head);
          if (f.ok) {
            bool skip_fiber = false;
            if (info) fprintf(stderr, "found fiber on line %d\n", lineno);
            if (rv.nfibers) {
              // check for doubles.
              for (int32_t k = 0; k < rv.nfibers; k++) {
                if (sv8equals(rv.fibers[k].name, f.name)) {
                  skip_fiber = true;
                  char buf[f.name.len+1];
                  memset(buf, 0, f.name.len+1);
                  memcpy(buf, f.name.data, f.name.len);
                  warn("a fiber named “%s” already exists; will be skipped", buf);
                }
              }
            }
            if (!skip_fiber) {
              // Store fiber in the fiber arena.
              *arena_new(&rv.fibera, Fiber, 1) = f;
              rv.nfibers++;
            }
          } else {
            warn("error reading fiber on line %d...", lineno);
          }
          break;
        case 'r':
          r = parse_resin(ccut.head);
          if (r.ok) {
            bool skip_resin = false;
            if (info) fprintf(stderr, "found resin on line %d\n", lineno);
            if (rv.nfibers) {
              // check for doubles
              for (int32_t k = 0; k < rv.nresins; k++) {
                if (sv8equals(rv.resins[k].name, f.name)) {
                  skip_resin = true;
                  char buf[f.name.len+1];
                  memset(buf, 0, f.name.len+1);
                  memcpy(buf, f.name.data, f.name.len);
                  warn("a resin named “%s” already exists; will be skipped", buf);
                }
              }
            }
            if (!skip_resin) {
              // Store fiber in the fiber arena.
              *arena_new(&rv.resina, Resin, 1) = r;
              rv.nresins++;
            }
          } else {
            warn("error reading resin on line %d...", lineno);
          }
          break;
        default:
          break;
      }
    }
    ccut = sv8cut(ccut.tail, '\n');
    lineno++;
  }
  return rv;
}
