// file: parser.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-09 15:53:32 +0200
// Last modified: 2025-08-09T15:55:27+0200

#pragma once

#include "arena.h"
#include "core.h"
#include "stringview.h"

typedef struct {
  Arena resina;
  Arena fibera;
  Resin *resins;
  Fiber *fibers;
  int32_t nresins, nfibers;
} FRdata;

typedef struct {
  Arena laminaa;
  Arena laminatesa;
  Lamina *laminas;
  Laminate *laminates;
  int32_t nlamina, nlaminates;
} Ldata;

#ifdef __cplusplus
extern "C" {
#endif

// parser.c
extern Sv8 read_file(char *path, Arena *permanent);
extern FRdata fibers_and_resins(Sv8 contents, bool info);
extern Ldata laminates(Sv8 contents, bool info, FRdata fr);

#ifdef __cplusplus
}
#endif
