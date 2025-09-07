// file: matrix.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2025-09-07 12:18:21 +0200
// Last modified: 2025-09-07T12:32:48+0200

#pragma once

#define LIMIT 1e-10

#include <stdbool.h>

// Inverts a matrix, if possible.
extern bool mat_inv2(double m[2][2], double i[2][2]);

// Cleans matrix a of small numbers, sets them to 0.
extern void mat_clean2(double a[2][2]);

// Calculate the determinant of the matrix.
// Uses mat_ref internally.
extern double mat_det5(double m[5][5]);

// Inverts a matrix if possible.
extern bool mat_inv6(double m[6][6], double i[6][6]);

// Multiplies two matrices
extern void mat_mul6(double a[6][6], double b[6][6],
                     double ab[6][6]);

// Copies matrix a into b
extern void mat_cpy6(double a[6][6], double b[6][6]);

// Cleans matrix a of small numbers, sets them to 0.
extern void mat_clean6(double a[6][6]);

// Transpose the matrix a.
extern void mat_xpose6(double a[6][6]);

// Calculate the determinant of the matrix.
// Uses mat_ref internally.
extern double mat_det6(double m[6][6]);

// Remove row r and column k from the matrix
extern void mat_delete(double a[6][6], double b[5][5], int r, int k);
