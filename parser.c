// file: parser.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-04 00:11:34 +0200
// Last modified: 2026-05-07T20:29:37+0200

#include "arena.h"
#include "logging.h"
#include "core.h"
#include "stringview.h"
#include "parser.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>  // for fopen.
#include <stdlib.h> // for exit.
#include <string.h> // for memset(3), memcpy(3)

static const Resin generic_resins[3] = {
  {RESN, 2900.0, 0.29, 40e-6, 1.15, SV8("generic-epoxy"), true},
  {RESN, 4000.0, 0.36, 40e-6, 1.20, SV8("generic-polyester"), true},
  {RESN, 3500.0, 0.36, 51.5e-6, 1.10, SV8("generic-vinylester"), true}
};

static const Fiber generic_fibers[3] = {
  {FIBR, 73000.0, 0.22, 5.3e-6, 2.60, SV8("generic-e-glass"), true},
  {FIBR, 230000.0, 0.27, -0.38e-6, 1.80, SV8("generic-carbon"), true},
  {FIBR, 124000.0, 0.36, -4.9e-6, 1.44, SV8("generic-aramid49"), true},
};

// Read the file into the permanent arena. Return a stringview to the
// contents.
static Sv8 read_file(char *path, Arena *permanent);
// Scan the file contents, allocate space for entities.
static ParseResult allocate(Sv8 contents, Arena *permanent);
static void fibers_and_resins(Sv8 contents, ParseResult *result);
static void laminates(Sv8 contents, ParseResult *result);

ParseResult parse_file(char *path, Arena *permanent)
{
  Sv8 contents = read_file(path, permanent);
  ParseResult rv = allocate(contents, permanent);
  fibers_and_resins(contents, &rv);
  laminates(contents, &rv);
  return rv;
}

Sv8 read_file(char *path, Arena *permanent)
{
  assert(path != 0);
  assert(permanent != 0);
  Sv8 contents = {0};
  FILE *inputfile = fopen(path, "r");
  if (inputfile == 0) {
    error("could not open file %s", path);
    exit(EXIT_FAILURE);
  }
  fseek(inputfile, 0L, SEEK_END);
  // Make space for extra newline.
  ptrdiff_t size = ftell(inputfile) + 1;
  rewind(inputfile);
  contents.data = arena_new(permanent, char, size);
  contents.len = size;
  ptrdiff_t rv = fread(contents.data, sizeof(char), size, inputfile);
  // Append extra newline.
  contents.data[rv++] = '\n';
  fclose(inputfile);
  if (rv != size) {
    info("file “%s” has size %td bytes, but only %td bytes read.", path, size, rv);
  }
  return contents;
}

ParseResult allocate(Sv8 contents, Arena *permanent)
{
  assert(permanent != 0);
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
        case 'f':
          rv.f++;
          break;
        case 'r':
          rv.r++;
          break;
        case 't':
          rv.t++;
          break;
        case 'l':
          rv.l++;
          break; // Comments are connected to lamina.
        case 'c':
          comments++;
          break;
        case 's':
          rv.s++;
          break;
      }
    }
    ccut = sv8cut(ccut.tail, '\n');
  }
  info("found %d fibers", rv.f);
  info("found %d resins", rv.r);
  info("found %d laminates", rv.t);
  info("found %d lamina", rv.l);
  info("found %d comments", comments);
  info("found %d symmetry lines", rv.s);
  // Worst case assumption: all laminates are mirrored.
  rv.l *= 2;
  // Add space for generics.
  rv.f += 3;
  rv.r += 3;
  // Allocate memory
  rv.resins = arena_new(permanent, Resin, rv.f);
  rv.fibers = arena_new(permanent, Fiber, rv.f);
  rv.laminas = arena_new(permanent, Lamina, rv.l);
  rv.laminates = arena_new(permanent, Laminate, rv.t);
  info("succesfully allocated memory.");
  rv.ok = true;
  return rv;
}

static Resin parse_resin(Sv8 line);
static Fiber parse_fiber(Sv8 line);

void fibers_and_resins(Sv8 contents, ParseResult *result)
{
  assert(result != 0);
  assert(result->resins != 0);
  assert(result->fibers != 0);
  result->ok = true;
  // Add generic resins
  memcpy(result->resins, generic_resins, 3 * sizeof(Resin));
  result->ru += 3;
  // Add generic resins
  memcpy(result->fibers, generic_fibers, 3 * sizeof(Fiber));
  result->fu += 3;
  int32_t lineno = 1;
  Sv8Cut ccut = sv8cut(contents, '\n');
  while (ccut.ok == true) {
    Fiber f = {0};
    Resin r = {0};
    Sv8 stripped = sv8strip(ccut.head);
    Sv8 parse_input = sv8lskip(stripped, 2);
    if (stripped.data[1] == ':') {
      switch (stripped.data[0]) {
        case 'f':
          f = parse_fiber(parse_input);
          if (f.ok) {
            bool skip_fiber = false;
            info("found fiber “%s” on line %d", sv8cstring(f.name), lineno);
            // Reject redefinitions of fiber names.
            for (int32_t k = 0; k < result->fu; k++) {
              if (sv8equals(result->fibers[k].name, f.name)) {
                skip_fiber = true;
                warning("a fiber named “%s” already exists; will be skipped", sv8cstring(f.name));
              }
            }
            if (!skip_fiber) {
              // Store fiber in the fiber arena.
              result->fibers[result->fu++] = f;
            }
          } else {
            warning("could not read fiber on line %d...", lineno);
          }
          break;
        case 'r':
          r = parse_resin(parse_input);
          if (r.ok) {
            bool skip_resin = false;
            info("found resin “%s” on line %d", sv8cstring(r.name), lineno);
            // Reject redefinition of resin names.
            for (int32_t k = 0; k < result->ru; k++) {
              if (sv8equals(result->resins[k].name, f.name)) {
                skip_resin = true;
                warning("a resin named “%s” already exists; will be skipped", sv8cstring(f.name));
              }
            }
            if (!skip_resin) {
              // Store resin in the resin arena.
              result->resins[result->ru++] = r;
            }
          } else {
            warning("could not read resin on line %d...", lineno);
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
  // line starts with the Young's modulus after possible whitespace.
  Sv8Double E = sv8tod(line);
  if (E.ok) {
    rv.E = E.result;
    rv.ok = true;
    //debug("E = %g", E.result);
  } else {
    return rv; // empty
  }
  // E.tail now starts with the Poisson constant after whitespace.
  Sv8Double ν = sv8tod(E.tail);
  if (ν.ok) {
    rv.ν = ν.result;
    rv.ok = true;
    //debug("ν = %g", ν.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ν.tail now starts with the CTE after whitespace.
  Sv8Double α = sv8tod(ν.tail);
  if (α.ok) {
    rv.α = α.result;
    rv.ok = true;
    //debug("α = %g", α.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // α.tail now starts with the density after whitespace.
  Sv8Double ρ = sv8tod(α.tail);
  if (ρ.ok) {
    rv.ρ = ρ.result;
    rv.ok = true;
    //debug("ρ = %g", ρ.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ρ.tail now starts with the name after whitespace.
  rv.name = sv8strip(ρ.tail);
  //debug("rv.name.len = %ld", rv.name.len);
  rv.ok = true;
  return rv;
}

Fiber parse_fiber(Sv8 line)
{
  Fiber rv = {0};
  rv.magic = FIBR;
  // line starts with the Young's modulus after possible whitespace.
  Sv8Double E1 = sv8tod(line);
  if (E1.ok) {
    rv.E1 = E1.result;
    rv.ok = true;
    //debug("E1 = %g", E1.result);
  } else {
    return rv; // empty
  }
  // E1.tail now starts with the Poisson constant after whitespace.
  Sv8Double ν12 = sv8tod(E1.tail);
  if (ν12.ok) {
    rv.ν12 = ν12.result;
    rv.ok = true;
    //debug("ν12 = %g", ν12.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ν12.tail now starts with the CTE after whitespace.
  Sv8Double α1 = sv8tod(ν12.tail);
  if (α1.ok) {
    rv.α1 = α1.result;
    rv.ok = true;
    //debug("α1 = %g", α1.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // α1.tail now starts with the density after whitespace.
  Sv8Double ρ = sv8tod(α1.tail);
  if (ρ.ok) {
    rv.ρ = ρ.result;
    rv.ok = true;
    //debug("ρ = %g", ρ.result);
  } else {
    rv.ok = false;
    return rv;
  }
  // ρ.tail now starts with the name after whitespace.
  rv.name = sv8strip(ρ.tail);
  rv.ok = true;
  return rv;
}

static Laminate *current_laminate(ParseResult *result);
static Laminate parse_t(Sv8 line, int32_t lineno);
static Sv8 parse_m(Sv8 line, int32_t lineno, ParseResult *result);
static Lamina parse_l(Sv8 line, int32_t lineno, ParseResult *result);

void laminates(Sv8 contents, ParseResult *result)
{
  char state = ' ';
  // valid states: ' ' = start, 'k' = skip invalid laminate, 't' = start laminate,
  // 'm' = matrix, 'l' = lamina, 'c' = comment, 's' = symmetry
  // Restart from the beginning.
  Sv8Cut ccut = sv8cut(contents, '\n');
  int32_t lineno = 1;
  Sv8 comment = {0};
  while (ccut.ok == true) {
    Sv8 stripped = sv8strip(ccut.head);
    Sv8 parse_input = sv8lskip(stripped, 2);
    if (stripped.data[1] == ':') {
      switch (stripped.data[0]) {
        case 't':
          // This resets the state machine.
          state = 't';
          // This laminate structure is empty exept for the name.
          Laminate lm = parse_t(parse_input, lineno);
          if (lm.ok) {
            // Check for existing laminate with the same name.
            bool skip_lm = false;
            for (int32_t k = 0; k < result->tu; k++) {
              if (sv8equals(result->laminates[k].name, lm.name)) {
                skip_lm = true;
                state = 'k'; // sKip m, l and s-lines.
                warning("a laminate named “%s” already exists on line %d, it will be skipped",
                        sv8cstring(lm.name), lineno);
              }
            }
            if (!skip_lm) {
              // Store the pointer to the next free lamina in the laminate.
              lm.layers = result->laminas + result->lu;
              // Copy the laminate into the laminate arena.
              result->laminates[result->tu++] = lm;
              info("found laminate named “%s” on line %d", sv8cstring(lm.name), lineno);
            }
          }
          break;
        case 'm':
          if (state == 'k') {
            warning("skipping m-line on line %d", lineno);
            break;
          }
          if (state != 't') {
            warning("unexpected m:-line on line %d; will be ignored", lineno);
            break;
          }
          state = 'm';
          Sv8 resin_name = parse_m(parse_input, lineno, result);
          if (resin_name.data == 0) {
            //warn("could not parse m-line on line %d", lineno);
            state = 'k';
            break;
          }
          bool unknown_resin = true;
          // pcurresin defined here?
          Resin *pcurresin = 0;
          for (int32_t k = 0; k < result->ru; k++) {
            if (sv8equals(result->resins[k].name, resin_name)) {
              unknown_resin = false;
              pcurresin = result->resins + k;
              break;
            }
          }
          Laminate *pcurlam = current_laminate(result);
          if (unknown_resin) {
            warning("resin “%s” does not exist on line %d", sv8cstring(resin_name), lineno);
            warning("skipping laminate “%s”", sv8cstring(pcurlam->name));
            // Delete laminate from arena.
            memset(result->laminates + --result->tu, 0, sizeof(Laminate));
            state = 'k';
          } else {
            // Now that we know the resin, store it and the vf in the
            // laminate.
            pcurlam->r = *pcurresin;
            info("using resin “%s” on line %d", sv8cstring(pcurlam->r.name), lineno);
          }
          break;
        case 'c':
          if (state != 'm' && state != 'l') {
            warning("unexpected c:-line on line %d; will be ignored", lineno);
            break;
          }
          state = 'l';
          // Save comment for addition to next lamina.
          comment = sv8strip(parse_input);
          //debug("storing comment “%s” on line %d", sv8cstring(comment), lineno);
          break;
        case 'l':
          if (state == 'k') {
            warning("skipping l-line on %d", lineno);
            break;
          }
          if (state == 's') {
            warning("l-line after an s-line on line %d; skipping", lineno);
            break;
          }
          if (state != 'm' && state != 'l') {
            warning("unexpected l:-line on line %d; will be ignored", lineno);
            break;
          }
          state = 'l';
          Lamina lmn = parse_l(parse_input, lineno, result);
          if (lmn.ok) {
            // Add comment if appliccable.
            if (comment.data && comment.len) {
              lmn.comment = comment;
              comment = (Sv8) {
                0, 0
              };
            }
            // Store the lamina
            debug("storing lamina on line %d", lineno);
            result->laminas[result->lu++] = lmn;
            pcurlam = current_laminate(result);
            pcurlam->nlayers++;
          }
          break;
        case 's':
          if (state == 'k') {
            warning("skipping s-line on %d", lineno);
          } else if (state != 'l') {
            warning("unexpected s:-line on line %d; will be ignored", lineno);
          } else {
            state = 's';
            // Remove comment, if any.
            comment = (Sv8) {
              0
            };
            info("found s-line on line %d", lineno);
            // Mirror the lamina.
            pcurlam = current_laminate(result);
            for (int32_t j = pcurlam->nlayers, k = result->lu - 1; j > 0 ; j--, k--) {
              result->laminas[result->lu] = result->laminas[k];
              if (j == pcurlam->nlayers) {
                result->laminas[result->lu].symm = true;
              }
              result->laminas[result->lu].comment = (Sv8) {
                0
              };
              result->lu++;
            }
            pcurlam->nlayers *= 2;
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

Laminate *current_laminate(ParseResult *result)
{
  Laminate *rv = result->laminates;
  if (result->tu > 0) {
    rv += result->tu - 1;
  }
  return rv;
}


Laminate parse_t(Sv8 line, int32_t lineno)
{
  Laminate rv = {0};
  rv.magic = LMNT;
  // line starts with the name after possible whitespace.
  rv.name = sv8strip(line);
  rv.ok = true;
  if (rv.name.len == 0) {
    warning("laminate without a name on line %d will be ignored", lineno);
    rv.ok = false;
  } else {
    debug("laminate name “%s” found on line %d", sv8cstring(rv.name), lineno);
  }
  return rv;
}

Sv8 parse_m(Sv8 line, int32_t lineno, ParseResult *result)
{
  Laminate *pcurlam = current_laminate(result);
  result->ok = false;
  // line starts with the fiber volume fraction after possible whitespace.
  Sv8Double vf = sv8tod(line);
  if (vf.ok) {
    pcurlam->vf = vf.result;
    debug("fiber volume fraction “%f” found on line %d", vf.result, lineno);
  } else {
    warning("m-line without fiber volume faction will be ignored on line %d", lineno);
  }
  // vf.tail should now contain the name of the resin.
  Sv8 resin_name = sv8strip(vf.tail);
  if (resin_name.len == 0) {
    warning("m-line without resin name on line %d will be ignored", lineno);
    return (Sv8) {
      0
    };
  }
  debug("resin named “%s” found on line %d", sv8cstring(resin_name), lineno);
  result->ok = false;
  return resin_name;
}

Lamina parse_l(Sv8 line, int32_t lineno, ParseResult *result)
{
  Laminate *pcurlam = current_laminate(result);
  Lamina rv = {0}; // This sets rv.ok to false...
  debug("line %d: “%s”", lineno, sv8cstring(line));
  // line starts with the fiber area weight after possible whitespace.
  Sv8Double area_weight = sv8tod(line);
  if (area_weight.ok) {
    rv.fiber_weight = area_weight.result;
    debug("lamina area weight %g g/m2 found on line %d", area_weight.result, lineno);
  } else {
    warning("reading lamina area weight failed on line %d", lineno);
    return rv;
  }
  Sv8Double angle = sv8tod(area_weight.tail);
  if (angle.ok) {
    rv.angle = angle.result;
    debug("lamina angle =  %g° on line %d", rv.angle, lineno);
  } else {
    warning("reading lamina angle failed on line %d", lineno);
    return rv;
  }
  Sv8Double vf = sv8tod(angle.tail);
  Sv8 next = angle.tail;
  double vfval = 0.01;
  if (vf.ok) {
    // found optional vf.
    debug("optional fiber volume fraction %g found on line %d", vf.result, lineno);
    vfval = vf.result;
    next = vf.tail;
  } else {
    vfval = pcurlam->vf;
  }
  Sv8 fiber_name = sv8strip(next);
  Fiber f = {0};
  if (fiber_name.len != 0) {
    // Look up the fiber name
    for (int32_t k = 0; k < result->fu; k++) {
      //debug("compare to “%s”", sv8cstring(result->fibers[k].name));
      if (sv8equals(result->fibers[k].name, fiber_name)) {
        f = result->fibers[k];
        f.ok = true;
        debug("found fiber name “%s” on line %d", sv8cstring(fiber_name), lineno);
        break;
      }
    }
    if (f.ok == false) {
      warning("fiber “%s” on line %d does not exist; skipping lamina",
              sv8cstring(fiber_name), lineno);
      return rv;
    }
  } else {
    debug("no fiber name found on line %d", lineno);
  }
  rv = init_lamina(f, pcurlam->r, area_weight.result, angle.result, vfval);
  return rv;
}
