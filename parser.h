// file: parser.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-09 15:53:32 +0200
// Last modified: 2026-02-11T07:07:36+0100

#pragma once

#include "arena.h"
#include "core.h"

typedef struct {
  Resin *resins;
  Fiber *fibers;
  Lamina *laminas;
  Laminate *laminates;
  int32_t f, r, t, m, l, c, s; // Allocated
  int32_t fu, ru, tu, mu, lu, cu, su; // Used
  bool ok;
} ParseResult;


#ifdef __cplusplus
extern "C" {
#endif

// parser.c
extern ParseResult parse_file(char *path, Arena *permanent, bool info);

#ifdef __cplusplus
}
#endif
