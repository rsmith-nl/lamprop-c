// file: types.h
// vim:fileencoding=utf-8:ft=cpp
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-03T19:31:50+0200
// Last modified: 2025-08-05T21:43:28+0200

#pragma once

#include "arena.h"
#include "stringview.h"

typedef struct {
  float E;
  float ν;
  float α;
  float ρ;
  Sv8 name;
} Resin;

typedef struct {
  float E1;
  float ν12;
  float α1;
  float ρ;
  Sv8 name;
} Fiber;

typedef struct {
  Fiber f;
  Resin r;
  float fiber_weight;
  float angle;
  float vf;
  float thickness;
  float resin_weight;
  float E1, E2, E3;
  float G12, G13, G23;
  float ν12, ν13, ν23;
  float αx, αy, αxy;
  float Q̅11, Q̅12, Q̅16, Q̅22, Q̅26, Q̅66, Q̅s44, Q̅s55, Q̅s45;
  float ρ;
  float C[6][6];
} Lamina;

typedef struct {
  Sv8 name;
  int32_t nlayers;
  Lamina *layers;
  float thickness;
  float fiber_weight, resin_weight;
  float ρ;
  float vf, wf;
  float ABD[6][6], abd[6][6];
  float H[2][2], h[2][2];
  float Ex, Ey, Ez;
  float Gxy, Gyz, Gxz;
  float νxy, νyx;
  float αx, αy;
  float C[6][6], S[6][6];
  float tEx, tEy, tEz;
  float tGxy, tGyz, tGxz;
  float tνxy, tνyx, tνyz;
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
