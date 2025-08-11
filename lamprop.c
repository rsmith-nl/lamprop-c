// file: lamprop.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:20:39+0200
// Last modified: 2025-08-11T20:25:35+0200

#include "core.h"
#include "logging.h"
#include "parser.h"
#include "setup.h"

#include <stdio.h>  // for fprintf(3)
#include <stdlib.h> // for abort(3)
#include <string.h> // for memset(3), memcpy(3)

// text.c
extern void text_out(Laminate *pl, bool eng, bool mat, bool fea);
// latex.c
extern void latex_out(Laminate *pl, bool eng, bool mat, bool fea);

int main(int argc, char *argv[])
{
  debug("starting lamprop...");
  Options opt = setup(argc, argv);
  debug("opt.argc = %d", opt.argc);
  debug("opt.argv = %s", opt.argv);
  // General allocation arena. Stores file contents.
  // This is also used as the storage for strings.
  if (opt.argv != 0) {
    Arena permanent = arena_create(33554432);
    Sv8 contents = read_file(opt.argv, &permanent);
    if (opt.info) {
      fprintf(stderr, "file “%s” is %td bytes.\n", opt.argv, contents.len);
    }
    ptrdiff_t nlines = sv8count(contents, '\n');
    if (opt.info) {
      fprintf(stderr, "file “%s” contains %td lines.\n", opt.argv, nlines);
    }
    // Scan for fibers and resins
    FRdata fr = fibers_and_resins(contents, opt.info);
    if (opt.info) {
      fprintf(stderr, "found %d fibers\n", fr.nfibers);
      fprintf(stderr, "found %d resins\n", fr.nresins);
    }
    // Scan for laminates
    Ldata ld = laminates(contents, opt.info, fr);
    if (opt.info) {
      fprintf(stderr, "found %d laminates\n", ld.nlaminates);
    }
    for (int32_t j = 0; j < ld.nlaminates; j++) {
      Laminate *pl = ld.laminates + j;
      if (!finish_laminate(pl)) {
        pl->magic = 0; // disable the laminate.
      }
      // TODO: print laminates and properties....
      switch (opt.output) {
        case TEXT:
          text_out(pl, opt.eng, opt.matrix, opt.fea);
          break;
        case LATEX:
          latex_out(pl, opt.eng, opt.matrix, opt.fea);
          break;
        case HTML:
          break;
      }
#ifndef NDEBUG
      if (pl->magic == LMNT) {
        debug("- laminate %d is a valid laminate", j+1);
        int32_t valid_la = 0;
        for (int32_t k = 0; k < pl->nlayers; k++) {
          Lamina *pa = pl->layers + k;
          if (pa->magic == LAYR) {
            valid_la++;
          }
        }
        debug("  it has %d/%d valid lamina", valid_la, pl->nlayers);
      }
#endif
    }
    // Clean up
    arena_destroy(&permanent);
    arena_destroy(&fr.resina);
    arena_destroy(&fr.fibera);
    arena_destroy(&ld.laminaa);
    arena_destroy(&ld.laminatesa);
  } else {
    fprintf(stderr, "WARNING: no laminate file name supplied.\n");
  }
  debug("ending lamprop normally...");
  return 0;
}
