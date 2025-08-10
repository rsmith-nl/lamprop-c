// file: core.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-09 12:21:26 +0200
// Last modified: 2025-08-10T09:10:14+0200

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






#include "matrix6.h"
#include "core.h"

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
  double Tbar[6][6] = {0}, Tbarx[6][6] = {0}, res[6][6] = {0}, C[6][6] = {0};
  tbar(Tbar, angle);
  mat_cpy6(Tbar, Tbarx);
  mat_xpose6(Tbarx);
  mat_mul6(Tbarx, Cp, res);
  mat_mul6(res, Tbar, C);
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
  // TODO: finish calculations.
  return true;
}
