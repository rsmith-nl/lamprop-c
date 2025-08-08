// file: lamprop.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:20:39+0200
// Last modified: 2025-08-08T14:40:39+0200

#include "arena.h"
#include "logging.h"
#include "stringview.h"
#include "types.h"
#include "setup.h"

#include <stdio.h>  // for fprintf(3)
#include <stdlib.h> // for abort(3)

#define NRESINS 100
#define NLAMINA 1000

int main(int argc, char *argv[])
{
  debug("starting lamprop...");
  Options opt = setup(argc, argv);
  debug("opt.argc = %d", opt.argc);
  debug("opt.argv = %s", opt.argv);
  if (opt.argv != 0) {
    Arena permanent = arena_create(33554432);
    Arena resina = arena_create(NRESINS*sizeof(Resin));
    Arena fibera = arena_create(NRESINS*sizeof(Fiber));
    //Arena lamina = arena_create(NLAMINA*sizeof(Lamina));
    Sv8 contents = read_file(opt.argv, &permanent);
    if (opt.info) fprintf(stderr, "file “%s” is %td bytes.\n", opt.argv, contents.len);
    ptrdiff_t nlines = sv8count(contents, '\n');
    if (opt.info) fprintf(stderr, "file “%s” contains %td lines.\n", opt.argv, nlines);
    int32_t fcnt = 0, rcnt = 0, lineno = 1;
    Sv8Cut ccut = sv8cut(contents, '\n');
    // Scan for fibers and resins
    while (ccut.ok == true) {
      if (ccut.head.data[1] == ':') {
        switch (ccut.head.data[0]) {
          case 'f':
            fcnt++;
            Fiber f = parse_fiber(ccut.head);
            if (f.ok) {
              if (opt.info) fprintf(stderr, "found fiber on line %d\n", lineno);
              // Store fiber in the fiber arena.
              *arena_new(&fibera, Fiber, 1) = f;
            } else {
              warn("error reading fiber on line %d...", lineno);
              fcnt--;
            }
            break;
          case 'r':
            rcnt++;
            Resin r = parse_resin(ccut.head);
            if (r.ok) {
              if (opt.info) fprintf(stderr, "found resin on line %d\n", lineno);
              // Store resin in the resin arena.
              *arena_new(&resina, Resin, 1) = r;
            } else {
              warn("error reading resin on line %d...", lineno);
              rcnt--;
            }
            break;
          default:
            break;
        }
      }
      ccut = sv8cut(ccut.tail, '\n');
      lineno++;
    }
    if (opt.info) {
      fprintf(stderr, "found %d fibers\n", fcnt);
      fprintf(stderr, "found %d resins\n", rcnt);
    }
    // Scan for laminates
    int32_t tcnt = 0, scnt = 0;
    char current = ' ';
    // Restart from the beginning.
    ccut = sv8cut(contents, '\n');
    lineno = 1;
    while (ccut.ok == true) {
      if (ccut.head.data[1] == ':') {
        switch (ccut.head.data[0]) {
          case 't':
            tcnt++;
            current = 't';
            if (opt.info) fprintf(stderr, "found t-line on line %d\n", lineno);
            break;
          case 'm':
            if (current != 't') {
              warn("unexpected m:-line; will be ignored");
            } else {
              current = 'm';
              if (opt.info) fprintf(stderr, "found m-line on line %d\n", lineno);
            }
            break;
          case 'l':
            if (current != 'm' && current != 'l') {
              warn("unexpected l:-line; will be ignored");
            } else {
              current = 'l';
              if (opt.info) fprintf(stderr, "found l-line on line %d\n", lineno);
            }
            break;
          case 's':
            if (current != 'l') {
              warn("unexpected s:-line; will be ignored");
            } else {
              scnt++;
              current = ' ';
              if (opt.info) fprintf(stderr, "found s-line on line %d\n", lineno);
            }
            break;
          default:
            break;
        }
      }
      ccut = sv8cut(ccut.tail, '\n');
      lineno++;
    }
    if (opt.info) {
      fprintf(stderr, "found %d laminates\n", tcnt);
      fprintf(stderr, "found %d symmetric laminates\n", scnt);
    }
  } else {
    fprintf(stderr, "WARNING: no laminate file name supplied.\n");
  }
  debug("ending lamprop...");
  return 0;
}
