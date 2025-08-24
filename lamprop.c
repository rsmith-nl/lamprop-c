// file: lamprop.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:20:39+0200
// Last modified: 2025-08-24T01:36:03+0200

#include "core.h"
#include "logging.h"
#include "parser.h"
#include "setup.h"

#include <stdio.h>  // for fprintf(3)

#define PASZ 33554432

// text.c
extern void text_out(Laminate *pl, bool eng, bool mat, bool fea);
// latex.c
extern void latex_out(Laminate *pl, bool eng, bool mat, bool fea);

int main(int argc, char *argv[])
{
  debug("starting lamprop...");
  Options opt = setup(argc, argv);
  while (opt.argc > 0) {
    debug("opt.argc = %d", opt.argc);
    debug("opt.argv[0] = %s", opt.argv[0]);
    // General allocation arena. Stores file contents.
    // This is also used as the storage for strings.
    Arena permanent = arena_create(PASZ);
    Sv8 contents = read_file(opt.argv[0], &permanent);
    if (opt.info) {
      fprintf(stderr, "file “%s” is %td bytes.\n", opt.argv[0], contents.len);
    }
    ptrdiff_t nlines = sv8count(contents, '\n');
    if (opt.info) {
      fprintf(stderr, "file “%s” contains %td lines.\n", opt.argv[0], nlines);
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
    if (opt.info) {
      ptrdiff_t used = permanent.cur - permanent.begin;
      fprintf(stderr, "#lamprop-c INFO: "
              "permanent arena, %td of %td bytes used\n", used, (ptrdiff_t)PASZ);
      used = (fr.resina.cur - fr.resina.begin)/sizeof(Resin);
      fprintf(stderr, "#lamprop-c INFO: "
              "resin arena, %td of %d resins used\n", used, NRESINS);
      used = (fr.fibera.cur - fr.fibera.begin)/sizeof(Fiber);
      fprintf(stderr, "#lamprop-c INFO: "
              "fiber arena, %td of %d fibers used\n", used, NRESINS);
      used = (ld.laminaa.cur - ld.laminaa.begin)/sizeof(Lamina);
      fprintf(stderr, "#lamprop-c INFO: "
              "lamina arena, %td of %d lamina used\n", used, NLAMINA);
      used = (ld.laminatesa.cur - ld.laminatesa.begin)/sizeof(Laminate);
      fprintf(stderr, "#lamprop-c INFO: "
              "laminate arena, %td of %d laminates used\n", used, NLAMINATES);
    }
    // Clean up
    arena_destroy(&permanent);
    arena_destroy(&fr.resina);
    arena_destroy(&fr.fibera);
    arena_destroy(&ld.laminaa);
    arena_destroy(&ld.laminatesa);
    // Advance
    opt.argv++;
    opt.argc--;
  } //else {
  //  fprintf(stderr, "WARNING: no laminate file name supplied.\n");
  //}
  debug("ending lamprop normally...");
  return 0;
}
