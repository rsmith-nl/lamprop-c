// file: setup.h
// vim:fileencoding=utf-8:ft=cpp:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-08 04:00:36 +0200
// Last modified: 2025-08-09T22:19:17+0200

#pragma once

#include <stdbool.h>

enum Output {
  TEXT = 0,
  LATEX,
  HTML
};

typedef struct {
  enum Output output;
  bool eng, matrix, fea, info;
  int argc;
  char *argv;
} Options;

#ifdef __cplusplus
extern "C" {
#endif

extern Options setup(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif
