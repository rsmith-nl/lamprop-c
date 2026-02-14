// file: parser.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-04 00:11:34 +0200
// Last modified: 2026-02-14T22:39:20+0100

#include "arena.h"
#include "logging.h"
#include "core.h"
#include "stringview.h"
#include "parser.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>  // for fopen
#include <string.h> // for memset(3), memcpy(3)

typedef struct {
  int32_t f, r, t, m, l, c, s;
} Counts;

static const Resin generic_resins[3] = {
  {RESN, 2900.0, 0.29, 40e-6, 1.15, SV8("generic-epoxy"), true},
  {RESN, 4000.0, 0.36, 40e-6, 1.20, SV8("generic-polyester"), true},
  {RESN, 3500.0, 0.36, 51.5e-6, 1.10, SV8("generic-vinylester"), true}
};

static const Fiber generic_fibers[3] = {
  {FIBR, 73000.0, 0.33, 5.3e-6, 2.60, SV8("generic-e-glas"), true},
  {FIBR, 230000.0, 0.27, -0.38e-6, 1.80, SV8("generic-carbon"), true},
  {FIBR, 124000.0, 0.36, -4.9e-6, 1.44, SV8("generic-aramid49"), true},
};

// Scan the file contents, count different entity types.
static Counts count_lines(Sv8 contents);
// Read the file into the permanent arena. Return a stringview to the
// contents.
static Sv8 read_file(char *path, Arena *permanent, bool info);
// Scan the file contents, allocate space for entities.
static ParseResult allocate(Sv8 contents, Arena *permanent, bool info);


ParseResult parse_file(char *path, Arena *permanent, bool info)
{
  Sv8 contents = read_file(path, permanent, info);
  ParseResult rv = allocate(contents, permanent, info);
  return rv;
}

Sv8 read_file(char *path, Arena *permanent, bool info)
{
  assert(path!=0);
  assert(permanent!=0);
  Sv8 contents = {0};
  FILE *inputfile = fopen(path, "r");
  if (inputfile==0) {
    return contents;
    if (info) {
      fprintf(stderr, "INFO: could not open file %s\n", path);
    }
  }
  fseek(inputfile, 0L, SEEK_END);
  ptrdiff_t size = ftell(inputfile);
  rewind(inputfile);
  contents.data = arena_new(permanent, char, size);
  contents.len = size;
  ptrdiff_t rv = fread(contents.data, sizeof(char), size, inputfile);
  fclose(inputfile);
  if (rv != size && info) {
    fprintf(stderr,
            "INFO: file “%s” has size %td bytes, but only %td bytes read.\n",
            path, size, rv);
  }
  return contents;
}

//Counts count_lines(Sv8 contents)
//{
//  Counts rv = {0};
//  Sv8Cut ccut = sv8cut(contents, '\n');
//  while (ccut.ok == true) {
//    Sv8 stripped = sv8strip(ccut.head);
//    if (stripped.data[1] == ':') {  // It is a command
//      switch (stripped.data[0]) {
//        case 'f': rv.f++; break;
//        case 'r': rv.r++; break;
//        case 't': rv.t++; break;
//        case 'm': rv.m++; break;
//        case 'l': rv.l++; break;
//        case 'c': rv.c++; break;
//        case 's': rv.s++; break;
//      }
//    }
//    ccut = sv8cut(ccut.tail, '\n');
//  }
//  // Assume all laminates are mirrored.
//  rv.l *= 2;
//  return rv;
//}

ParseResult allocate(Sv8 contents, Arena *permanent, bool info)
{
  assert(permanent!=0);
  ParseResult rv = {0};
  if (contents.data == 0 && contents.len == 0) {
    return rv; // shortcut om empty input.
  }
  int comments = 0;
  Sv8Cut ccut = sv8cut(contents, '\n');
  while (ccut.ok == true) {
    Sv8 stripped = sv8strip(ccut.head);
    if (stripped.data[1] == ':') {  // It is a command
      switch (stripped.data[0]) {
        case 'f': rv.f++; break;
        case 'r': rv.r++; break;
        case 't': rv.t++; break;
        case 'm': rv.m++; break;
        case 'l': rv.l++; break; // Comments are connected to lamina.
        case 'c': comments++; break;
        case 's': rv.s++; break;
      }
    }
    ccut = sv8cut(ccut.tail, '\n');
  }
  if (info) {
    fprintf(stderr, "INFO: found %d fibers\n", rv.f);
    fprintf(stderr, "INFO: found %d resins\n", rv.r);
    fprintf(stderr, "INFO: found %d laminates\n", rv.t);
    fprintf(stderr, "INFO: found %d matrices\n", rv.m);
    fprintf(stderr, "INFO: found %d lamina\n", rv.l);
    fprintf(stderr, "INFO: found %d comments\n", comments);
    fprintf(stderr, "INFO: found %d symmetry lines\n", rv.l);
  }
  // Worst case assumption: all laminates are mirrored.
  rv.l *= 2;
  // Worst case assumption: choose largest of m or t.
  rv.t = rv.m>rv.t?rv.m:rv.t;
  rv.m = rv.t;
  // Allocate memory
  rv.resins = arena_new(permanent, Resin, rv.f);
  rv.fibers = arena_new(permanent, Fiber, rv.f);
  rv.laminas = arena_new(permanent, Lamina, rv.l);
  rv.laminates = arena_new(permanent, Laminate, rv.t);
  if (info) {
    fprintf(stderr, "INFO: succesfully allocated memory.\n");
  }
  rv.ok = true;
  return rv;
}

static Resin parse_resin(Sv8 line);
static Fiber parse_fiber(Sv8 line);

void fibers_and_resins(Sv8 contents, ParseResult *result, bool info)
{
  assert(result!=0);
  result->ok = true;
  // Add generic resins
  memcpy(result->resins, generic_resins, 3*sizeof(Resin));
  result->ru += 3;
  // Add generic resins
  memcpy(result->fibers, generic_fibers, 3*sizeof(Fiber));
  result->fu += 3;
  int32_t lineno = 1;
  Sv8Cut ccut = sv8cut(contents, '\n');
  while (ccut.ok == true) {
    Fiber f = {0};
    Resin r = {0};
    Sv8 stripped = sv8strip(ccut.head);
    if (stripped.data[1] == ':') {
      switch (stripped.data[0]) {
        case 'f':
          f = parse_fiber(stripped);
          if (f.ok) {
            bool skip_fiber = false;
            if (info) {
              fprintf(stderr, "found fiber on line %d\n", lineno);
            }
            // Reject redefinitions of fiber names.
            for (int32_t k = 0; k < result->fu; k++) {
              if (sv8equals(result->fibers[k].name, f.name)) {
                skip_fiber = true;
                warn("a fiber named “%s” already exists; will be skipped", sv8cstring(f.name));
              }
            }
            if (!skip_fiber) {
              // Store fiber in the fiber arena.
              result->fibers[result->fu++] = f;
            }
          } else {
            warn("error reading fiber on line %d...", lineno);
          }
          break;
        case 'r':
          r = parse_resin(stripped);
          if (r.ok) {
            bool skip_resin = false;
            if (info) {
              fprintf(stderr, "found resin on line %d\n", lineno);
            }
            // Reject redefinition of resin names.
            for (int32_t k = 0; k < result->ru; k++) {
              if (sv8equals(result->resins[k].name, f.name)) {
                skip_resin = true;
                warn("a resin named “%s” already exists; will be skipped", sv8cstring(f.name));
              }
            }
            if (!skip_resin) {
              // Store resin in the resin arena.
              result->resins[result->ru++] = r;
            }
          } else {
            warn("error reading resin on line %d...", lineno);
          }
          break;
        default:
          break;
      }
    }
    ccut = sv8cut(ccut.tail, '\n');
    lineno++;
  }
}

Resin parse_resin(Sv8 line)
{
  Resin rv = {0};
  rv.magic = RESN;
  // This function is only called when *line* starts with 'r:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
  // cut.tail now starts with the Young's modulus after whitespace.
  Sv8Double E = sv8tod(cut.tail);
  if (E.ok) {
    rv.E = E.result;
    rv.ok = true;
    //debug("E = %g\n", E.result);
  } else {
    return rv; // empty
  }
  // E.tail now starts with the Poisson constant after whitespace.
  Sv8Double ν = sv8tod(E.tail);
  if (ν.ok) {
    rv.ν = ν.result;
    rv.ok = true;
    //debug("ν = %g\n", ν.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ν.tail now starts with the CTE after whitespace.
  Sv8Double α = sv8tod(ν.tail);
  if (α.ok) {
    rv.α = α.result;
    rv.ok = true;
    //debug("α = %g\n", α.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // α.tail now starts with the density after whitespace.
  Sv8Double ρ = sv8tod(α.tail);
  if (ρ.ok) {
    rv.ρ = ρ.result;
    rv.ok = true;
    //debug("ρ = %g\n", ρ.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ρ.tail now starts with the name after whitespace.
  rv.name = sv8strip(ρ.tail);
  //debug("rv.name.len = %ld\n", rv.name.len);
  rv.ok = true;
  return rv;
}

Fiber parse_fiber(Sv8 line)
{
  Fiber rv = {0};
  rv.magic = FIBR;
  // This function is only called when *line* starts with 'f:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
  // cut.tail now starts with the Young's modulus after whitespace.
  Sv8Double E1 = sv8tod(cut.tail);
  if (E1.ok) {
    rv.E1 = E1.result;
    rv.ok = true;
    //debug("E1 = %g\n", E1.result);
  } else {
    return rv; // empty
  }
  // E1.tail now starts with the Poisson constant after whitespace.
  Sv8Double ν12 = sv8tod(E1.tail);
  if (ν12.ok) {
    rv.ν12 = ν12.result;
    rv.ok = true;
    //debug("ν12 = %g\n", ν12.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ν12.tail now starts with the CTE after whitespace.
  Sv8Double α1 = sv8tod(ν12.tail);
  if (α1.ok) {
    rv.α1 = α1.result;
    rv.ok = true;
    //debug("α1 = %g\n", α1.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // α1.tail now starts with the density after whitespace.
  Sv8Double ρ = sv8tod(α1.tail);
  if (ρ.ok) {
    rv.ρ = ρ.result;
    rv.ok = true;
    //debug("ρ = %g\n", ρ.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ρ.tail now starts with the name after whitespace.
  rv.name = sv8strip(ρ.tail);
  rv.ok = true;
  return rv;
}

static Laminate parse_t(Sv8 line);
static Sv8 parse_m(Sv8 line, Laminate *pcurlam, ParseResult *result);
static Lamina parse_l(Sv8 line, Laminate *pcurlam, ParseResult *result);

void laminates(Sv8 contents, ParseResult *result, bool info)
{
  char state = ' ';
  // valid states: ' ' = start, 'k' = skip invalid laminate, 't' = start laminate,
  // 'm' = matrix, 'l' = lamina, 'c' = comment, 's' = symmetry
  // Restart from the beginning.
  Sv8Cut ccut = sv8cut(contents, '\n');
  int32_t lineno = 1;
  Resin *pcurresin = 0;
  Laminate *pcurlam = 0;
  while (ccut.ok == true) {
    if (ccut.head.data[1] == ':') {
      switch (ccut.head.data[0]) {
        case 't':
          // This resets the state machine.
          state = 't';
          // This laminate structure is empty exept for the name.
          Laminate lm = parse_t(ccut.head);
          if (lm.ok) {
            // Check for existing laminate with the same name.
            bool skip_lm = false;
            for (int32_t k = 0; k < result->tu; k++) {
              if (sv8equals(result->laminates[k].name, lm.name)) {
                skip_lm = true;
                state = 'k'; // sKip m, l and s-lines.
                warn("a laminate named “%s” already exists; will be skipped",
                     sv8cstring(lm.name));
              }
            }
            pcurlam = 0;
            if (!skip_lm) {
              // Store the current pointer to the lamina in the laminate.
              lm.layers = (void*)(result->laminas+result->lu);
              // Allocate and copy the laminate into the laminate arena.
              result->laminates[result->tu] = lm;
              pcurlam = result->laminates + result->tu;
              result->tu++;
              if (info) {
                fprintf(stderr, "found laminate named “%s” on line %d\n",
                        sv8cstring(lm.name), lineno);
              }
            }
          } else {
            warn("error reading laminate on line %d...", lineno);
          }
          break;
        case 'm':
          if (state == 'k') {
            warn("skipping m-line");
            break;
          }
          if (state != 't') {
            warn("unexpected m:-line; will be ignored");
            break;
          }
          state = 'm';
          Sv8 resin_name = parse_m(ccut.head, pcurlam, result);
          if (resin_name.data == 0) {
            warn("could not parse m-line on line %d", lineno);
            state = 'k';
            break;
          }
          bool unknown_resin = true;
          for (int32_t k = 0; k < result->ru; k++) {
            if (sv8equals(result->resins[k].name, resin_name)) {
              unknown_resin = false;
              pcurresin = result->resins + k;
              break;
            }
          }
          if (unknown_resin) {
            warn("resin “%s” does not exist; skipping laminate “%s”",
                  sv8cstring(resin_name), sv8cstring(pcurlam->name));
            // Delete laminate from arena.
            memset(result->laminates + --result->lu, 0, sizeof(Laminate));
            state='k';
          } else {
            // Now that we know the resin, store it and the vf in the
            // laminate.
            pcurlam->r = *pcurresin;
            pcurlam->vf = ml.vf;
            if (info) {
              fprintf(stderr, "using resin “%s” on line %d\n",
                      sv8cstring(ml.resin_name), lineno);
            }
          }
          break;
        case 'l':
          if (state == 'k') {
            warn("skipping l-line on %d", lineno);
            break;
          }
          if (state == 's') {
            warn("l-line after an s-line on line %d; skipping", lineno);
            break;
          }
          if (state != 'm' && state != 'l') {
            warn("unexpected l:-line on line %d; will be ignored", lineno);
            break;
          }
          state = 'l';
          Lamina lmn = parse_l(ccut.head, pcurlam, result);
          if (lmn.ok) {
            bool unknown_fiber = true;
            Fiber *pf = 0;
            for (int32_t k = 0; k < result->fu; k++) {
              if (sv8equals(result->fibers[k].name, lmn.fiber_name)) {
                unknown_fiber = false;
                pf = fr.fibers + k;
                break;
              }
            }
            if (unknown_fiber) {
              warn("fiber “%s” on line %d does not exist; skipping lamina",
                    sv8cstring(lmn.fiber_name), lineno);
              state='l';
            } else {
              // Fill lamina properties
              Lamina la = init_lamina(*pf, *pcurresin, lmn.area_weight,
                                      lmn.angle, lmn.optvf?lmn.vf:pcurlam->vf);
              // Store lamina in the arena.
              *arena_new(&rv.laminaa, Lamina, 1) = la;
              rv.nlamina++;
              if (info) {
                fprintf(stderr, "using fiber “%s” on line %d\n",
                        sv8cstring(lmn.fiber_name), lineno);
              }
            }
          } else {
            warn("could not parse l-line on line %d", lineno);
          }
          break;
        case 's':
          if (state == 'k') {
            warn("skipping s-line on %d", lineno);
          } else if (state != 'l') {
            warn("unexpected s:-line on line %d; will be ignored", lineno);
          } else {
            state = 's';
            Lamina *begin = pcurlam->layers;
            Lamina *end = begin + (pcurlam->nlayers -1);
            for (Lamina *p = end; p >= begin; p--) {
              *arena_new(&rv.laminaa, Lamina, 1) = *p;
              pcurlam->nlayers++;
            }
            if (info) {
              fprintf(stderr, "found s-line on line %d\n", lineno);
            }
          }
          break;
        default:
          break;
      }
    }
    ccut = sv8cut(ccut.tail, '\n');
    lineno++;
  }
  return rv;
}

Laminate parse_t(Sv8 line)
{
  Laminate rv = {0};
  rv.magic = LMNT;
  // This function is only called when *line* starts with 't:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
  // cut.tail now starts with the name after whitespace.
  rv.name = sv8strip(cut.tail);
  //debug("rv.name = %s\n", sv8ctring(rv.name));
  rv.ok = true;
  if (rv.name.len==0) {
    warn("laminate without a name found");
    rv.ok = false;
  }
  return rv;
}

Sv8 parse_m(Sv8 line, Laminate *pcurlam, ParseResult *result)
{
  assert(pcurlam!=0);
  result->ok = false;
  // This function is only called when *line* starts with 'm:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
  // cut.tail now starts with the fiber volume fraction after whitespace.
  Sv8Double vf = sv8tod(cut.tail);
  if (vf.ok) {
    pcurlam->vf = vf.result;
    pcurlam->vf = rv.vf;
  }
  // vf.tail should now contain the name of the resin.
  Sv8 resin_name = sv8strip(vf.tail);
  if (resin_name.len != 0) {
    return (Sv8){0};
  }
  return resin_name;
}

Lamina parse_l(Sv8 line, Laminate *pcurlam, ParseResult *result)
{
  assert(pcurlam!=0);
  Lamina rv = {0};
  // This function is only called when *line* starts with 'l:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
  // cut.tail now starts with the fiber area weight after whitespace.
  Sv8Double area_weight = sv8tod(cut.tail);
  if (area_weight.ok) {
    rv.fiber_weight = area_weight.result;
  } else {
    //debug("reading area weight failed");
    return rv;
  }
  Sv8Double angle = sv8tod(area_weight.tail);
  if (angle.ok) {
    rv.angle = angle.result;
    //debug("rv.angle = %g", rv.angle);
  } else {
    //debug("reading angle failed");
    return rv;
  }
  Sv8Double vf = sv8tod(angle.tail);
  Sv8 next = angle.tail;
  if (vf.ok) {
    // found optional vf.
    rv.vf = vf.result;
    next = vf.tail;
  } else {
    rv.vf = pcurlam->vf;
  }
  Sv8 fiber_name = sv8strip(next);
  if (fiber_name.len != 0) {
    rv.fiber_name = fiber_name;
    //debug("rv.fiber_name = %s", sv8cstring(rv.fiber_name));
    rv.ok = true;
  } //else {
  //debug("fiber name empty");
  //}
  // Increase the layer count in the laminate.
  pcurlam->nlayers++;
  return rv;
}
