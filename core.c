// file: core.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-09 12:21:26 +0200
// Last modified: 2025-09-07T12:38:08+0200

// Core functions of lamprop.
//
// The following references were used in coding this module:
//
// @Book{Barbero:2018,
//     author = {Ever J. Barbero},
//     title = {Introduction to composite materials design},
//     edition   = 3,
//     publisher = {CRC Press},
//     year = {2018},
//     isbn = {9781138196803},
//     note = {hardcover}
// }
//
// @Book{Barbero:2008,
//     author = {Ever J. Barbero},
//     title = {Finite element analysis of composite materials},
//     publisher = {CRC Press},
//     year = {2008},
//     isbn = {9781420054330},
//     note = {hardcover}
// }
//
// @Book{Bower:2010,
//     author = {Allan F. Bower},
//     title = {Applied Mechanics of Solids},
//     publisher = {CRC Press},
//     year = {2010},
//     isbn = {9781439802472a},
//     note = {hardcover}
// }
//
// @Book{Hyer:1998,
//   author =       {Micheal W. Hyer},
//   title =        {Stress analysis of fiber-reinforced composite materials},
//   publisher =    {McGraw--Hill},
//   year =         {1998},
//   isbn =         {0071159835}
// }
//
// @Book{Tsai:1992,
//   author =       {Stephen W. Tsai},
//   title =        {Theory of composites design},
//   publisher =    {Think Composites},
//   year =         {1992},
//   isbn =         {0961809035}
// }
//
// @Article{1992WeiEn..52...29H,
//    author = {Hart-Smith, L.~J.},
//     title = "{The ten-percent rule for preliminary sizing of fibrous
//                   composite structures}",
//   journal = {Weight Engineering},
//      year = 1992,
//    volume = 52,
//     pages = {29-45},
//   adsnote = {Provided by the Smithsonian/NASA Astrophysics Data System}
// }
//
// @Book{Vinson:1987,
//   author =       {J.R. Vinson},
//   title =        {The behavior of structures composed of composite materials},
//   publisher =    {Martinus Nijhoff Publishers},
//   year =         {1987},
//   isbn =         {90247312590}
//   note =         {hardcover}
// }
//
// @Techreport{Nettles:1994,
//   author =       {A.T. Nettles},
//   title =        {Basic Mechanics of Laminated Plates},
//   institution =  {NASA},
//   year =         {1994},
//   number =       {Reference Publication 1351}
// }

#include "matrix.h"
#include "core.h"

#include <stdbool.h>
#include <string.h>
#include <math.h>

// Generate rotation angle matrix.
static void tbar(double out[6][6], double angle);

Lamina init_lamina(Fiber f, Resin r, double area_weight, double angle, double vf)
{
  Lamina rv = {0};
  rv.magic = LAYR;
  rv.f = f;
  rv.fiber_weight = area_weight;
  angle = M_PI * angle / 180.0;  // convert to radians.
  rv.angle = angle;
  rv.vf = vf;
  double vm = 1 - vf;
  double fiber_thickness = area_weight / (f.ρ * 1000);
  rv.thickness = fiber_thickness * (1 + vm / vf);
  rv.resin_weight = rv.thickness * vm * r.ρ * 1000;  // Resin [g/m²]
  rv.E1 = vf * f.E1 + r.E * vm;  // Hyer:1998, p. 115, (3.32)
  const double ξ = 1.5;  // Giner, 2014
  double η = (f.E1 / r.E - 1) / (f.E1 / r.E + ξ);
  rv.E2 = r.E * ((1 + ξ * η * vf) / (1 - η * vf));  // Barbero:2018, p. 117
  rv.E3 = rv.E2;  // Assumed for UD layers.
  rv.ν13 = rv.ν12 = f.ν12 * vf + r.ν * vm;  // Barbero:2018, p. 118
  // The matrix-dominated cylindrical assemblage model is used for G12.
  double Gm = r.E / (2 * (1 + r.ν));
  rv.G12 = Gm * (1 + vf) / (1 - vf);
  rv.G13 = rv.G12;
  double ν21 = rv.ν12 * rv.E2 / rv.E1;  // Nettles:1994, p. 4
  // Calculate G23, necessary for Qs44.
  double Kf = f.E1 / (3 * (1 - 2 * f.ν12));
  double Km = r.E / (3 * (1 - 2 * r.ν));
  double K = 1 / (vf / Kf + vm / Km);
  double ν23 = 1 - ν21 - rv.E2 / (3 * K);
  rv.ν23 = ν23;
  rv.G23 = rv.E2 / (2 * (1 + ν23));  // Barbero:2008, p. 23, Barbero:2018, p. 504
  double m = cos(angle), n = sin(angle);
  // Calculate the 3D stiffness matrix for this lamina
  // Note about terminology: in the literature, the stiffness matrix is
  // generally named C, while its inverse the compliance matrix is called S.
  // This is confusing IMO, but I will follow convention here for the sake of
  // clarity.
  // First, the compliance matrix in lamina coordinates
  double Sp[6][6] = {
    {1 / rv.E1, -rv.ν12 / rv.E1, -rv.ν13 / rv.E1, 0, 0, 0},
    {-rv.ν12 / rv.E1, 1 / rv.E2, -rv.ν23 / rv.E2, 0, 0, 0},
    {-rv.ν13 / rv.E1, -rv.ν23 / rv.E2, 1 / rv.E3, 0, 0, 0},
    {0, 0, 0, 1 / rv.G23, 0, 0},
    {0, 0, 0, 0, 1 / rv.G13, 0},
    {0, 0, 0, 0, 0, 1 / rv.G12},
  };
  double Cp[6][6] = {0};
  // Invert it to the stiffness matrix in lamina coordinates
  mat_inv6(Sp, Cp);
  // Convert to global coordinates.
  double Tbar[6][6] = {0}, Tbarx[6][6] = {0}, res[6][6] = {0};
  tbar(Tbar, angle);
  mat_cpy6(Tbar, Tbarx);
  mat_xpose6(Tbarx);
  mat_mul6(Tbarx, Cp, res);
  mat_mul6(res, Tbar, rv.C);
  // The powers of the sine and cosine are often used later.
  double m2 = m * m;
  double m3 = m2 * m, m4 = m2 * m2;
  double n2 = n * n;
  double n3 = n2 * n, n4 = n2 * n2;
  // Calculate CTE
  double α1 = (f.α1 * f.E1 * vf + r.α * r.E * vm) / rv.E1;
  // Since α2 properties of fibers are hard to come by, we have to estimate.
  // This is based on our own measurements.
  double α2 = vf * r.α;  // This is not 100% accurate, but simple.
  rv.αx = α1 * m2 + α2 * n2;
  rv.αy = α1 * n2 + α2 * m2;
  rv.αxy = 2 * (α1 - α2) * m * n;
  // Barbero:2018, p. 159
  double denum = 1 - rv.ν12 * ν21;
  double Q11 = rv.E1 / denum, Q12 = rv.ν12 * rv.E2 / denum;
  double Q22 = rv.E2 / denum, Q66 = rv.G12;
  double Qs44 = rv.G23;
  double Qs55 = rv.G12;  // Assuming transverse isotropy.
  // Q̅ according to Hyer:1997, p. 182
  rv.Q̅11 = Q11 * m4 + 2 * (Q12 + 2 * Q66) * n2 * m2 + Q22 * n4;
  double QA = Q11 - Q12 - 2 * Q66;
  double QB = Q12 - Q22 + 2 * Q66;
  rv.Q̅12 = (Q11 + Q22 - 4 * Q66) * n2 * m2 + Q12 * (n4 + m4);
  rv.Q̅16 = QA * n * m3 + QB * n3 * m;
  rv.Q̅22 = Q11 * n4 + 2 * (Q12 + 2 * Q66) * n2 * m2 + Q22 * m4;
  rv.Q̅26 = QA * n3 * m + QB * n * m3;
  rv.Q̅66 = (Q11 + Q22 - 2 * Q12 - 2 * Q66) * n2 * m2 + Q66 * (n4 + m4);
  // Q̅star (Q̅s) according to Barbero:2018, p. 167
  rv.Q̅s44 = Qs44 * m2 + Qs55 * n2;
  rv.Q̅s55 = Qs44 * n2 + Qs55 * m2;
  rv.Q̅s45 = (rv.Q̅s55 - rv.Q̅s44) * n * m;
  // Calculate density
  rv.ρ = f.ρ * vf + r.ρ * vm;
  rv.ok = true;
  return rv;
}

void tbar(double out[6][6], double angle)
{
  double c = cos(angle), s = sin(angle);
  double Tbar[6][6] = {
    {c * c, s * s, 0, 0, 0, c * s},
    {s * s, c * c, 0, 0, 0, -c * s},
    {0, 0, 1, 0, 0, 0},
    {0, 0, 0, c, -s, 0},
    {0, 0, 0, s, c, 0},
    {-2 * c * s, 2 * c * s, 0, 0, 0, c * c - s * s}
  };
  memcpy(out, Tbar, 6*6*sizeof(double));
}


bool finish_laminate(Laminate *pl)
{
  if (pl->magic!=LMNT) {
    return false;
  }
  if (pl->nlayers == 0) {
    return false;
  }
  double thickness = 0.0;
  double fiber_weight = 0.0;
  double ρ = 0.0;
  double vf = 0.0;
  double resin_weight = 0.0;
  for (int32_t j = 0; j < pl->nlayers; j++) {
    thickness += pl->layers[j].thickness;
    fiber_weight += pl->layers[j].fiber_weight;
    ρ += pl->layers[j].ρ * pl->layers[j].thickness;
    vf += pl->layers[j].vf * pl->layers[j].thickness;
    resin_weight += pl->layers[j].resin_weight;
  }
  ρ /= thickness;
  vf /= thickness;
  double wf = fiber_weight / (fiber_weight + resin_weight);
  // Store values in laminate data structure
  pl->thickness = thickness;
  pl->fiber_weight = fiber_weight;
  pl->resin_weight = resin_weight;
  pl->ρ = ρ;
  pl->vf = vf;
  pl->wf = wf;
  // Calculate C for total laminate
  double zs = -thickness / 2;
  double lz2[pl->nlayers], lz3[pl->nlayers];
  double C[6][6] = {0};
  for (int32_t j = 0; j < pl->nlayers; j++) {
    double ze = zs + pl->layers[j].thickness;
    lz2[j] = (ze * ze - zs * zs) / 2;
    lz3[j] = (ze * ze * ze - zs * zs * zs) / 3;
    zs = ze;
    double scale = pl->layers[j].thickness / thickness;
    for (int32_t r = 0; r < 6; r++) {
      for (int32_t c = 0; c < 6; c++) {
        C[r][c] += pl->layers[j].C[r][c] * scale;
      }
    }
  }
  // Cleanse C from numbers close to 0.
  mat_clean6(C);
  double S[6][6] = {0};
  mat_inv6(C, S);
  // Store matrices.
  memcpy(pl->C, C, 6*6*sizeof(double));
  memcpy(pl->S, S, 6*6*sizeof(double));
  // Calculate ABD and H matrices
  double ABD[6][6] = {0};
  double H[2][2] = {0};
  double Ntx = 0.0, Nty = 0.0, Ntxy = 0.0, c3 = 0.0;
  for (int32_t j = 0; j < pl->nlayers; j++) {
    // first row
    ABD[0][0] += pl->layers[j].Q̅11 * pl->layers[j].thickness;  // Hyer:1998, p. 290
    ABD[0][1] += pl->layers[j].Q̅12 * pl->layers[j].thickness;
    ABD[0][2] += pl->layers[j].Q̅16 * pl->layers[j].thickness;
    ABD[0][3] += pl->layers[j].Q̅11 * lz2[j];
    ABD[0][4] += pl->layers[j].Q̅12 * lz2[j];
    ABD[0][5] += pl->layers[j].Q̅16 * lz2[j];
    // second row
    ABD[1][0] += pl->layers[j].Q̅12 * pl->layers[j].thickness;
    ABD[1][1] += pl->layers[j].Q̅22 * pl->layers[j].thickness;
    ABD[1][2] += pl->layers[j].Q̅26 * pl->layers[j].thickness;
    ABD[1][3] += pl->layers[j].Q̅12 * lz2[j];
    ABD[1][4] += pl->layers[j].Q̅22 * lz2[j];
    ABD[1][5] += pl->layers[j].Q̅26 * lz2[j];
    // third row
    ABD[2][0] += pl->layers[j].Q̅16 * pl->layers[j].thickness;
    ABD[2][1] += pl->layers[j].Q̅26 * pl->layers[j].thickness;
    ABD[2][2] += pl->layers[j].Q̅66 * pl->layers[j].thickness;
    ABD[2][3] += pl->layers[j].Q̅16 * lz2[j];
    ABD[2][4] += pl->layers[j].Q̅26 * lz2[j];
    ABD[2][5] += pl->layers[j].Q̅66 * lz2[j];
    // fourth row
    ABD[3][0] += pl->layers[j].Q̅11 * lz2[j];
    ABD[3][1] += pl->layers[j].Q̅12 * lz2[j];
    ABD[3][2] += pl->layers[j].Q̅16 * lz2[j];
    ABD[3][3] += pl->layers[j].Q̅11 * lz3[j];
    ABD[3][4] += pl->layers[j].Q̅12 * lz3[j];
    ABD[3][5] += pl->layers[j].Q̅16 * lz3[j];
    // fifth row
    ABD[4][0] += pl->layers[j].Q̅12 * lz2[j];
    ABD[4][1] += pl->layers[j].Q̅22 * lz2[j];
    ABD[4][2] += pl->layers[j].Q̅26 * lz2[j];
    ABD[4][3] += pl->layers[j].Q̅12 * lz3[j];
    ABD[4][4] += pl->layers[j].Q̅22 * lz3[j];
    ABD[4][5] += pl->layers[j].Q̅26 * lz3[j];
    // sixth row
    ABD[5][0] += pl->layers[j].Q̅16 * lz2[j];
    ABD[5][1] += pl->layers[j].Q̅26 * lz2[j];
    ABD[5][2] += pl->layers[j].Q̅66 * lz2[j];
    ABD[5][3] += pl->layers[j].Q̅16 * lz3[j];
    ABD[5][4] += pl->layers[j].Q̅26 * lz3[j];
    ABD[5][5] += pl->layers[j].Q̅66 * lz3[j];
    // Calculate unit thermal stress resultants.
    // Hyer:1998, p. 445
    Ntx += (pl->layers[j].Q̅11 * pl->layers[j].αx +
            pl->layers[j].Q̅12 * pl->layers[j].αy +
            pl->layers[j].Q̅16 * pl->layers[j].αxy) * pl->layers[j].thickness;
    Nty += (pl->layers[j].Q̅12 * pl->layers[j].αx +
            pl->layers[j].Q̅22 * pl->layers[j].αy +
            pl->layers[j].Q̅26 * pl->layers[j].αxy) * pl->layers[j].thickness;
    Ntxy += (pl->layers[j].Q̅16 * pl->layers[j].αx +
             pl->layers[j].Q̅26 * pl->layers[j].αy +
             pl->layers[j].Q̅66 * pl->layers[j].αxy) * pl->layers[j].thickness;
    // Calculate H matrix (derived from Barbero:2018, p. 181)
    // Note: division by 4 moved to end because of accuracy!
    double sb = 5 * (pl->layers[j].thickness - 4 * lz3[j] / (thickness*thickness)) / 4;
    H[0][0] += pl->layers[j].Q̅s44 * sb;
    H[0][1] += pl->layers[j].Q̅s45 * sb;
    H[1][0] += pl->layers[j].Q̅s45 * sb;
    H[1][1] += pl->layers[j].Q̅s55 * sb;
    // Calculate E3
    c3 += pl->layers[j].thickness / pl->layers[j].E3;
  }
  // Finish the matrices, discarding very small numbers in ABD and H.
  mat_clean6(ABD);
  mat_clean2(H);
  // Store matrices.
  memcpy(pl->ABD, ABD, 6*6*sizeof(double));
  memcpy(pl->H, H, 2*2*sizeof(double));
  // Calculate inverted matrices
  double abd[6][6] = {0};
  double h[2][2] = {0};
  mat_inv6(ABD, abd);
  mat_inv2(H, h);
  // Store inverted matrices.
  memcpy(pl->abd, abd, 6*6*sizeof(double));
  memcpy(pl->h, h, 2*2*sizeof(double));
  // Calculate the engineering properties.
  // Nettles:1994, p. 34 e.v.
  double dABD = mat_det6(ABD);
  double tmp[5][5] = {0};
  double dt1 = 0, dt2 = 0, dt3 = 0, dt4 = 0, dt5 = 0;
  mat_delete(ABD, tmp, 0, 0);
  dt1 = mat_det5(tmp);
  pl->Ex = dABD/(dt1 * thickness);
  mat_delete(ABD, tmp, 1, 1);
  dt2 = mat_det5(tmp);
  pl->Ey = dABD/(dt2 * thickness);
  mat_delete(ABD, tmp, 2, 2);
  dt3 = mat_det5(tmp);
  pl->Gxy = dABD/(dt3 * thickness);
  mat_delete(ABD, tmp, 0, 1);
  dt4 = mat_det5(tmp);
  mat_delete(ABD, tmp, 1, 0);
  dt5 = mat_det5(tmp);
  pl->νxy = dt4 / dt1;
  pl->νyx = dt5 / dt2;
  // See Barbero:2018, p. 197
  pl->Gyz = H[0][0] / thickness;
  pl->Gxz = H[1][1] / thickness;
  // All layers experience the same force in Z-direction.
  pl->Ez = thickness / c3;
  // Calculate and store the coefficients of thermal expansion.
  // *Technically* only valid for a symmetric laminate!
  // Hyer:1998, p. 451, (11.86)
  pl->αx = abd[0][0] * Ntx + abd[0][1] * Nty + abd[0][2] * Ntxy;
  pl->αy = abd[1][0] * Ntx + abd[1][1] * Nty + abd[1][2] * Ntxy;
  // Calculate and store tensor engineering properties
  pl->tEx = 1 / S[0][0];
  pl->tEy = 1 / S[1][1];
  pl->tEz = 1 / S[2][2];
  pl->tGxy  = 1 / S[5][5];
  pl->tGxz = 1 / S[4][4];
  pl->tGyz = 1 / S[3][3];
  pl->tνxy = -S[1][0] / S[0][0];
  pl->tνxz = -S[2][0] / S[0][0];
  pl->tνyz = -S[2][1] / S[1][1];
  return true;
}


bool isortho(double src[6][6])
{
  int32_t r[24] = {0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5};
  int32_t c[24] = {3, 4, 5, 3, 4, 5, 3, 4, 5, 0, 1, 2, 4, 5, 0, 1, 2, 3, 5, 0, 1, 2, 3, 4};
  int32_t count = 0;
  for (int32_t x = 0; x < 24; x++) {
    if (fabs(src[r[x]][c[x]]) < LIMIT) {
      count++;
    };
  }
  if (count == 24) {
    return true;
  }
  return false;
}

void toabaqusi(double src[6][6], double dest[6][6])
{
  for (int32_t r = 0; r < 6; r++) {
    for (int32_t c = 0; c < 6; c++) {
      dest[r][c] = src[r][c] * 1e6;
    }
  }
  // Exceptions
  dest[0][3] = src[0][5] * 1e6;
  dest[0][5] = src[0][3] * 1e6;
  dest[1][3] = src[1][5] * 1e6;
  dest[1][5] = src[1][3] * 1e6;
  dest[2][3] = src[2][5] * 1e6;
  dest[2][5] = src[2][3] * 1e6;
  dest[3][3] = src[5][5] * 1e6;
  dest[5][5] = src[3][3] * 1e6;
}
