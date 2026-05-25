// file: lamprop.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:20:39+0200
// Last modified: 2026-05-25T02:20:10+0200

#include "core.h"
#include "logging.h"
#include "parser.h"
#include "setup.h"

#include <stdlib.h>

#ifdef _WIN32
#include <io.h> // for _setmode
// instead of including windows.h....
extern int __stdcall SetConsoleOutputCP(unsigned int);
#endif

#define PASZ 33554432

// text.c
extern void text_out(Laminate *pl, Options opts);
// latex.c
extern void latex_out(Laminate *pl, Options opts);
// html.c
extern void html_out(Laminate *pl, Options opts);

int main(int argc, char *argv[])
{
#ifdef _WIN32
  _setmode(0, 0x8000);
  _setmode(1, 0x8000);
  SetConsoleOutputCP(65001);
#endif
  Options opt = setup(argc, argv);
  debug("starting lamprop...");
  debug("opt.argc = %d", opt.argc);
  debug("opt.argv[0] = %s", opt.argv[0]);
  if (opt.argc == 0) {
    error("no input filename given");
    return EXIT_FAILURE;
  }
  if (opt.output_filename != 0) {
    opt.outfile = fopen(opt.output_filename, "w");
    if (opt.outfile == 0) {
      return EXIT_FAILURE;
    }
  } else {
    // No filename given, use stdout.
    opt.outfile = stdout;
  }
  // General allocation arena. Stores file contents.
  // This is also used as the storage for strings.
  Arena permanent = arena_create(PASZ);
  ParseResult file_result = parse_file(opt.argv[0], &permanent);
  for (int32_t j = 0; j < file_result.tu; j++) {
    Laminate *pl = file_result.laminates + j;
    if (!finish_laminate(pl)) {
      continue; // skip the laminate.
    }
    switch (opt.output) {
      case TEXT:
        text_out(pl, opt);
        break;
      case LATEX:
        latex_out(pl, opt);
        break;
      case HTML:
        html_out(pl, opt);
        break;
    }
#ifndef NDEBUG
    if (pl->magic == LMNT) {
      debug("- laminate %d is a valid laminate", j + 1);
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
  if (opt.output_filename != 0) {
    fclose(opt.outfile);
  }
  info("permanent arena, %d of %d bytes used", permanent.current_offset, PASZ);
  info("%d of %d resins used", file_result.ru, file_result.r);
  info("%d of %d fibers used", file_result.fu, file_result.f);
  info("%d of %d laminates used", file_result.tu, file_result.t);
  info("%d of %d lamina used", file_result.lu, file_result.l);
  // Clean up
  arena_destroy(&permanent);
  debug("ending lamprop normally...");
  return EXIT_SUCCESS;
}
