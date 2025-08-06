// file: types.h
// vim:fileencoding=utf-8:ft=cpp
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:31:50+0200
// Last modified: 2025-08-06T19:45:51+0200

#pragma once

#include "arena.h"
#include "stringview.h"

#include <stdbool.h>

typedef struct {
  double E;
  double ν;
  double α;
  double ρ;
  Sv8 name;
  bool ok;
} Resin;

typedef struct {
  double E1;
  double ν12;
  double α1;
  double ρ;
  Sv8 name;
  bool ok;
} Fiber;

typedef struct {
  Fiber f;
  Resin r;
  double fiber_weight;
  double angle;
  double vf;
  double thickness;
  double resin_weight;
  double E1, E2, E3;
  double G12, G13, G23;
  double ν12, ν13, ν23;
  double αx, αy, αxy;
  double Q̅11, Q̅12, Q̅16, Q̅22, Q̅26, Q̅66, Q̅s44, Q̅s55, Q̅s45;
  double ρ;
  double C[6][6];
  bool ok;
} Lamina;

typedef struct {
  Sv8 name;
  int32_t nlayers;
  Lamina *layers;
  double thickness;
  double fiber_weight, resin_weight;
  double ρ;
  double vf, wf;
  double ABD[6][6], abd[6][6];
  double H[2][2], h[2][2];
  double Ex, Ey, Ez;
  double Gxy, Gyz, Gxz;
  double νxy, νyx;
  double αx, αy;
  double C[6][6], S[6][6];
  double tEx, tEy, tEz;
  double tGxy, tGyz, tGxz;
  double tνxy, tνyx, tνyz;
  bool ok;
} Laminate;

#ifdef __cplusplus
extern "C" {
#endif

extern Sv8 read_file(char *path, Arena *permanent);

extern Resin parse_resin(Sv8 line);
extern Fiber parse_fiber(Sv8 line);

#ifdef __cplusplus
}
#endif
