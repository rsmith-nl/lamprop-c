// file: lamprop.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:20:39+0200
// Last modified: 2025-08-06T23:17:28+0200

#include "arena.h"
#include "logging.h"
#include "stringview.h"
#include "types.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>  // for fprintf(3)
#include <stdlib.h> // for abort(3)
#include <string.h>

#undef fatal
#define fatal(...)                                                  \
  fprintf(stderr, "ERROR %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__);                                     \
  abort()

int main(int argc, char *argv[])
{
  debug("starting lamprop...\n");
  Arena perm = arena_create(16777216);
  Arena resina = arena_create(1048576);
  Arena fibera = arena_create(1048576);
  // Arena lamina = arena_create(10485760);
  if (argc > 1) {
    Sv8 contents = read_file(argv[argc-1], &perm);
    info("contents of “%s” is %td bytes long.", argv[argc-1], contents.len);
    ptrdiff_t nlines = sv8count(contents, '\n');
    info("file “%s” contains %td lines.", argv[argc-1], nlines);
    int32_t fcnt = 0, rcnt = 0;
    Sv8Cut ccut = sv8cut(contents, '\n');
    // Scan for fibers and resins
    while (ccut.ok == true) {
      if (ccut.head.data[1] == ':') {
        switch (ccut.head.data[0]) {
          case 'f':
            fcnt++;
            Fiber f = parse_fiber(ccut.head);
            if (f.ok) {
              // Store fiber in the fiber arena.
              *arena_new(&fibera, Fiber, 1) = f;
            } else {
              warn("Error reading fiber...");
              fcnt--;
            }
            break;
          case 'r':
            rcnt++;
            Resin r = parse_resin(ccut.head);
            if (r.ok) {
              // Store resin in the resin arena.
              *arena_new(&resina, Resin, 1) = r;
            } else {
              warn("Error reading resin...");
              rcnt--;
            }
            break;
          default:
            break;
        }
      }
      ccut = sv8cut(ccut.tail, '\n');
    }
    info("found %d fibers", fcnt);
    info("found %d resins", rcnt);
    // Scan for laminates
    int32_t tcnt = 0, scnt = 0;
    char current = ' ';
    ccut = sv8cut(contents, '\n');
    while (ccut.ok == true) {
      if (ccut.head.data[1] == ':') {
        switch (ccut.head.data[0]) {
          case 't':
            tcnt++;
            current = 't';
            info("found t-line");
            break;
          case 'm':
            if (current != 't') {
              fprintf(stderr, "WARNING: unexpected m:-line; will be ignored");
            } else {
              current = 'm';
              info("found m-line");
            }
            break;
          case 'l':
            if (current != 'm' && current != 'l') {
              fprintf(stderr, "WARNING: unexpected l:-line; will be ignored");
            } else {
              current = 'l';
              info("found l-line");
            }
            break;
          case 's':
            if (current != 'l') {
              fprintf(stderr, "WARNING: unexpected s:-line; will be ignored");
            } else {
              scnt++;
              current = ' ';
              info("found s-line");
            }
            break;
          default:
            break;
        }
      }
      ccut = sv8cut(ccut.tail, '\n');
    }
    info("found %d laminates", tcnt);
    info("found %d symmetric laminates", scnt);
  } else {
    fprintf(stderr, "ERROR: no laminate file name supplied.\n");
  }
  debug("ending lamprop...\n");
  return 0;
}
