// file: core.h
// vim:fileencoding=utf-8:ft=cpp:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-09 15:41:55 +0200
// Last modified: 2025-08-10T13:01:33+0200

#pragma once

#include "arena.h"
#include "stringview.h"

#include <stdbool.h>

#define NRESINS 1000
#define NLAMINATES 100
// Largest laminate I've ever used was 250 layers.
#define NLAMINA 25000

// Magic values;
#define FIBR 0x46494252
#define RESN 0x5245534e
#define LAYR 0x4c415952
#define LMNT 0x4c4d4e54

typedef struct {
  uint32_t magic;
  double E;
  double ν;
  double α;
  double ρ;
  Sv8 name;
  bool ok;
} Resin;

typedef struct {
  uint32_t magic;
  double E1;
  double ν12;
  double α1;
  double ρ;
  Sv8 name;
  bool ok;
} Fiber;

typedef struct {
  uint32_t magic;
  Fiber f;
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
  uint32_t magic;
  Sv8 name;
  int32_t nlayers;
  Lamina *layers;
  Resin r;
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
  double tνxy, tνxz, tνyz;
  bool ok;
} Laminate;


#ifdef __cplusplus
extern "C" {
#endif

// Initialze and return a Lamina structure.
extern Lamina init_lamina(Fiber f, Resin r, double area_weight, double angle, double vf);
extern bool finish_laminate(Laminate *pl);

#ifdef __cplusplus
}
#endif
