// file: parser.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-04 00:11:34 +0200
// Last modified: 2025-08-09T01:35:29+0200

#include "arena.h"
#include "stringview.h"
#include "logging.h"
#include "types.h"

#include <stdio.h>  // for fopen
#include <string.h> // for memset(3), memcpy(3)

Sv8 read_file(char *path, Arena *permanent)
{
  Sv8 contents = {0};
  FILE *inputfile = fopen(path, "r");
  if (inputfile==0) {
    return contents;
  }
  fseek(inputfile, 0L, SEEK_END);
  ptrdiff_t size = ftell(inputfile);
  rewind(inputfile);
  contents.data = arena_new(permanent, char, size);
  contents.len = size;
  ptrdiff_t rv = fread(contents.data, sizeof(char), size, inputfile);
  fclose(inputfile);
  if (rv != size) {
    fprintf(stderr,
            "WARNING: file “%s” has size %td bytes, but only %td bytes read.\n",
            path, size, rv);
  }
  return contents;
}

Resin parse_resin(Sv8 line)
{
  Resin rv = {0};
  // This function is only called when *line* starts with 'f:'.
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

Laminate parse_laminate(Sv8 line)
{
  Laminate rv = {0};
  rv.ok = true;
  rv.finished = false;
  // This function is only called when *line* starts with 't:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
  // cut.tail now starts with the name after whitespace.
  rv.name = sv8strip(cut.tail);
  if (rv.name.len==0) {
    warn("laminate without a name found");
    rv.ok = false;
  }
  return rv;
}

Mline parse_m(Sv8 line)
{
  Mline rv = {0};
  // This function is only called when *line* starts with 'm:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
  // cut.tail now starts with the fiber volume fraction after whitespace.
  Sv8Double vf = sv8tod(cut.tail);
  if (vf.ok) {
    rv.vf = vf.result;
  } else {
    return rv;
  }
  // vf.tail should now contain the name of the resin.
  Sv8 resin_name = sv8strip(vf.tail);
  if (resin_name.len != 0) {
    rv.resin_name = resin_name;
    rv.ok = true;
  }
  return rv;
}

Lline parse_l(Sv8 line)
{
  Lline rv = {0};
  // This function is only called when *line* starts with 'l:'.
  // So discard that.
  Sv8Cut cut = sv8lsplit(line);
  // cut.tail now starts with the fiber area weight after whitespace.
  Sv8Double area_weight = sv8tod(cut.tail);
  if (area_weight.ok) {
    rv.area_weight = area_weight.result;
  } else {
    return rv;
  }
  Sv8Double angle = sv8tod(area_weight.tail);
  if (angle.ok) {
    rv.angle = angle.result;
  } else {
    return rv;
  }
  // angle.tail should now contain the name of the fiber.
  Sv8 fiber_name = sv8strip(angle.tail);
  if (fiber_name.len != 0) {
    rv.fiber_name = fiber_name;
    rv.ok = true;
  }
  return rv;
}

FRdata fibers_and_resins(Sv8 contents, bool info)
{
  FRdata rv = {0};
  rv.resina = arena_create(NRESINS*sizeof(Resin));
  rv.resins = (void*)rv.resina.begin;
  rv.fibera = arena_create(NRESINS*sizeof(Fiber));
  rv.fibers = (void*)rv.fibera.begin;
  int32_t lineno = 1;
  Sv8Cut ccut = sv8cut(contents, '\n');
  Fiber f = {0};
  Resin r = {0};
  while (ccut.ok == true) {
    if (ccut.head.data[1] == ':') {
      switch (ccut.head.data[0]) {
        case 'f':
          f = parse_fiber(ccut.head);
          if (f.ok) {
            bool skip_fiber = false;
            if (info) {
              fprintf(stderr, "found fiber on line %d\n", lineno);
            }
            // check for doubles.
            for (int32_t k = 0; k < rv.nfibers; k++) {
              if (sv8equals(rv.fibers[k].name, f.name)) {
                skip_fiber = true;
                char buf[f.name.len+1];
                memset(buf, 0, f.name.len+1);
                memcpy(buf, f.name.data, f.name.len);
                warn("a fiber named “%s” already exists; will be skipped", buf);
              }
            }
            if (!skip_fiber) {
              // Store fiber in the fiber arena.
              *arena_new(&rv.fibera, Fiber, 1) = f;
              rv.nfibers++;
            }
          } else {
            warn("error reading fiber on line %d...", lineno);
          }
          break;
        case 'r':
          r = parse_resin(ccut.head);
          if (r.ok) {
            bool skip_resin = false;
            if (info) {
              fprintf(stderr, "found resin on line %d\n", lineno);
            }
            // check for doubles
            for (int32_t k = 0; k < rv.nresins; k++) {
              if (sv8equals(rv.resins[k].name, f.name)) {
                skip_resin = true;
                char buf[f.name.len+1];
                memset(buf, 0, f.name.len+1);
                memcpy(buf, f.name.data, f.name.len);
                warn("a resin named “%s” already exists; will be skipped", buf);
              }
            }
            if (!skip_resin) {
              // Store fiber in the fiber arena.
              *arena_new(&rv.resina, Resin, 1) = r;
              rv.nresins++;
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
  return rv;
}

Ldata laminates(Sv8 contents, bool info, FRdata fr)
{
  Ldata rv = {0};
  char state = ' ';
  rv.laminatesa = arena_create(NLAMINATES*sizeof(Laminate));
  rv.laminates = (void*)rv.laminatesa.begin;
  rv.laminaa = arena_create(NLAMINA*sizeof(Lamina));
  rv.laminas = (void*)rv.laminaa.begin;
  // Restart from the beginning.
  Sv8Cut ccut = sv8cut(contents, '\n');
  int32_t lineno = 1;
  while (ccut.ok == true) {
    if (ccut.head.data[1] == ':') {
      switch (ccut.head.data[0]) {
        case 't':
          state = 't'; // This resets the state.
          Laminate lm = parse_laminate(ccut.head);
          if (lm.ok) {
            // check for doubles
            bool skip_lm = false;
            for (int32_t k = 0; k < rv.nlaminates; k++) {
              if (sv8equals(rv.laminates[k].name, lm.name)) {
                skip_lm = true;
                char buf[lm.name.len+1];
                memset(buf, 0, lm.name.len+1);
                memcpy(buf, lm.name.data, lm.name.len);
                warn("a laminate named “%s” already exists; will be skipped", buf);
              }
            }
            if (!skip_lm) {
              // Store laminate in the laminate arena.
              *arena_new(&rv.laminatesa, Laminate, 1) = lm;
              rv.nlaminates++;
              if (info) {
                char tmpnm[lm.name.len+1];
                memset(tmpnm, 0, lm.name.len+1);
                memcpy(tmpnm, lm.name.data, lm.name.len);
                fprintf(stderr, "found laminate named “%s” on line %d\n", tmpnm, lineno);
              }
            }
          } else {
            warn("error reading laminate on line %d...", lineno);
          }
          break;
        case 'm':
          if (state != 't') {
            warn("unexpected m:-line; will be ignored");
          } else {
            state = 'm';
            Mline ml = parse_m(ccut.head);
            if (ml.ok) {
              bool unknown_resin = true;
              for (int32_t k = 0; k < fr.nresins; k++) {
                if (sv8equals(fr.resins[k].name, ml.resin_name)) {
                  unknown_resin = false;
                  break;
                }
              }
              char buf[ml.resin_name.len+1];
              memset(buf, 0, ml.resin_name.len+1);
              memcpy(buf, ml.resin_name.data, ml.resin_name.len);
              if (unknown_resin) {
                Laminate *curlam = rv.laminates + (rv.nlaminates-1);
                char buf2[curlam->name.len+1];
                memset(buf2, 0, curlam->name.len+1);
                memcpy(buf2, curlam->name.data, curlam->name.len);
                warn("resin “%s” does not exist; skipping laminate “%s”", buf, buf2);
                // Delete laminate from arena.
                rv.nlaminates--;
                rv.laminatesa.cur -= sizeof(Laminate);
                memset(rv.laminatesa.cur, 0, sizeof(Laminate));
                state=' ';
              } else {
                if (info) {
                  fprintf(stderr, "using resin “%s” on line %d\n", buf, lineno);
                }
              }
            } else {
              warn("could not parse m-line on line %d", lineno);
            }
          }
          break;
        case 'l':
          if (state != 'm' && state != 'l') {
            warn("unexpected l:-line on line %d; will be ignored", lineno);
          } else {
            state = 'l';
            Lline lmn = parse_l(ccut.head);
            if (lmn.ok) {
              bool unknown_fiber = true;
              for (int32_t k = 0; k < fr.nfibers; k++) {
                if (sv8equals(fr.fibers[k].name, lmn.fiber_name)) {
                  unknown_fiber = false;
                  break;
                }
              }
              char buf[lmn.fiber_name.len+1];
              memset(buf, 0, lmn.fiber_name.len+1);
              memcpy(buf, lmn.fiber_name.data, lmn.fiber_name.len);
              if (unknown_fiber) {
                warn("fiber “%s” does not exist; skipping lamina", buf);
                state='l';
              } else {
                // TODO: store lamina in arena.
                if (info) {
                  fprintf(stderr, "using fiber “%s” on line %d\n", buf, lineno);
                }
              }
            } else {
              warn("could not parse m-line on line %d", lineno);
            }
          }
          break;
        case 's':
          if (state != 'l') {
            warn("unexpected s:-line on line %d; will be ignored", lineno);
          } else {
            state = ' ';
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

