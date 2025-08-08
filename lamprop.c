// file: lamprop.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:20:39+0200
// Last modified: 2025-08-08T20:30:18+0200

#include "arena.h"
#include "logging.h"
#include "stringview.h"
#include "types.h"
#include "setup.h"

#include <stdio.h>  // for fprintf(3)
#include <stdlib.h> // for abort(3)
#include <string.h> // for memset(3), memcpy(3)

#define NLAMINATES 100
// Largest laminate I've ever used was 250 layers.
#define NLAMINA 25000


int main(int argc, char *argv[])
{
  debug("starting lamprop...");
  Options opt = setup(argc, argv);
  debug("opt.argc = %d", opt.argc);
  debug("opt.argv = %s", opt.argv);
  // General allocation arena. Stores file contents.
  // This is also used as the storage for strings.
  Arena permanent = arena_create(33554432);
  //Arena lamina = arena_create(NLAMINA*sizeof(Lamina));
  // Arena for laminates. Also basically an array
  Arena laminatesa = arena_create(NLAMINATES*sizeof(Laminate));
  //Laminate *laminates = (void*)laminatesa.begin;
  // If we need to add lamina to a laminate, we need the current laminate.
  int32_t curlam = -1;
  if (opt.argv != 0) {
    Sv8 contents = read_file(opt.argv, &permanent);
    if (opt.info) fprintf(stderr, "file “%s” is %td bytes.\n", opt.argv, contents.len);
    ptrdiff_t nlines = sv8count(contents, '\n');
    if (opt.info) fprintf(stderr, "file “%s” contains %td lines.\n", opt.argv, nlines);
    // Scan for fibers and resins
    FRdata fr = fibers_and_resins(contents, opt.info);
    if (opt.info) {
      fprintf(stderr, "found %d fibers\n", fr.nfibers);
      fprintf(stderr, "found %d resins\n", fr.nresins);
    }
    // Scan for laminates
    char state = ' ';
    // Restart from the beginning.
    Sv8Cut ccut = sv8cut(contents, '\n');
    int32_t lineno = 1;
    while (ccut.ok == true) {
      if (ccut.head.data[1] == ':') {
        switch (ccut.head.data[0]) {
          case 't':
            state = 't';
            Laminate lm = parse_laminate(ccut.head);
            if (lm.ok) {
              if (opt.info) {
                char tmpnm[lm.name.len+1];
                memset(tmpnm, 0, lm.name.len+1);
                memcpy(tmpnm, lm.name.data, lm.name.len);
                fprintf(stderr, "found t-line named “%s” on line %d\n", tmpnm, lineno);
              }
              // TODO: check for duplicates and ignore them.
              // Store laminate in the laminate arena.
              *arena_new(&laminatesa, Laminate, 1) = lm;
              curlam++;
            } else {
              warn("error reading laminate on line %d...", lineno);
            }
            break;
          case 'm':
            if (state != 't') {
              warn("unexpected m:-line; will be ignored");
            } else {
              state = 'm';
              Mline ml = parse_m(ccut.head);
              if (ml.ok) {
                if (opt.info) fprintf(stderr, "found m-line on line %d\n", lineno);
                // TODO: check that named resin exists.
              }
            }
            break;
          case 'l':
            if (state != 'm' && state != 'l') {
              warn("unexpected l:-line; will be ignored");
            } else {
              state = 'l';
              if (opt.info) fprintf(stderr, "found l-line on line %d\n", lineno);
            }
            break;
          case 's':
            if (state != 'l') {
              warn("unexpected s:-line; will be ignored");
            } else {
              state = ' ';
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
      fprintf(stderr, "found %d laminates\n", curlam);
      //fprintf(stderr, "found %d symmetric laminates\n", scnt);
    }
  } else {
    fprintf(stderr, "WARNING: no laminate file name supplied.\n");
  }
  debug("ending lamprop normally...");
  return 0;
}


