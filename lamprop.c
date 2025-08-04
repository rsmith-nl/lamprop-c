// file: lamprop.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:20:39+0200
// Last modified: 2025-08-04T22:07:28+0200

#include "arena.h"
#include "logging.h"
#include "stringview.h"

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

// From parser.c
extern Sv8 read_file(char *path, Arena *permanent);

int main(int argc, char *argv[])
{
  debug("starting lamprop...\n");
  Arena perm = arena_create(16777216);
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
            break;
          case 'r':
            rcnt++;
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
    int32_t tcnt = 0, mcnt = 0, lcnt = 0, scnt = 0;
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
              mcnt++;
              current = 'm';
              info("found m-line");
            }
            break;
          case 'l':
            if (current != 'm' && current != 'l') {
              fprintf(stderr, "WARNING: unexpected l:-line; will be ignored");
            } else {
              lcnt++;
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
    info("found %d laminate matrix", mcnt);
    info("found %d lamina", lcnt);
    info("found %d symmetries", scnt);
  } else {
    fprintf(stderr, "ERROR: no laminate file name supplied.\n");
  }
  debug("ending lamprop...\n");
  return 0;
}
