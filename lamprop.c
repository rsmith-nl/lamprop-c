// file: lamprop.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:20:39+0200
// Last modified: 2025-08-08T23:41:32+0200

#include "arena.h"
#include "logging.h"
#include "stringview.h"
#include "types.h"
#include "setup.h"

#include <stdio.h>  // for fprintf(3)
#include <stdlib.h> // for abort(3)
#include <string.h> // for memset(3), memcpy(3)

int main(int argc, char *argv[])
{
  debug("starting lamprop...");
  Options opt = setup(argc, argv);
  debug("opt.argc = %d", opt.argc);
  debug("opt.argv = %s", opt.argv);
  // General allocation arena. Stores file contents.
  // This is also used as the storage for strings.
  Arena permanent = arena_create(33554432);
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
    Ldata ld = laminates(contents, opt.info, fr);
    if (opt.info) {
      fprintf(stderr, "found %d laminates\n", ld.nlaminates);
    }
  } else {
    fprintf(stderr, "WARNING: no laminate file name supplied.\n");
  }
  debug("ending lamprop normally...");
  return 0;
}


