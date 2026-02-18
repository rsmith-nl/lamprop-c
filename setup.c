// file: setup.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
//
// Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: MIT
// Created: 2025-08-08 04:00:37 +0200
// Last modified: 2026-02-19T00:15:08+0100

#include "setup.h"
#include "logging.h"
#include "version.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>

const char license[] =
  "Copyright © 2025 R.F. Smith <rsmith@xs4all.nl>. All rights reserved.\n"
  "\n"
  "Redistribution and use in source and binary forms, with or without\n"
  "modification, are permitted provided that the following conditions\n"
  "are met:\n"
  "1. Redistributions of source code must retain the above copyright\n"
  "   notice, this list of conditions and the following disclaimer.\n"
  "2. Redistributions in binary form must reproduce the above copyright\n"
  "   notice, this list of conditions and the following disclaimer in the\n"
  "   documentation and/or other materials provided with the distribution.\n"
  "\n"
  "THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND\n"
  "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
  "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n"
  "ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE\n"
  "FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
  "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n"
  "OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n"
  "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n"
  "LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n"
  "OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n"
  "SUCH DAMAGE.\n";

const char help[] =
  "usage: lamprop [-h] [-l | -H] [-e] [-m] [-f] [-L | -v] [--log=(debug|info|warn|error|crit)]\n"
  "               [file ...]\n"
  "\n"
  "Calculate the elastic properties of a fibrous composite laminate. See the manual (lamprop-manual.pdf)\n"
  "for more in-depth information.\n"
  "\n"
  "positional arguments:\n"
  "  file                  one or more files to process\n"
  "\n"
  "options:\n"
  "  -h, --help            show this help message and exit\n"
  "  -l, --latex           generate LaTeX output (the default is plain text)\n"
  "  -H, --html            generate HTML output\n"
  "  -e, --eng             output only the engineering properties\n"
  "  -m, --mat             output only the ABD matrix and stiffness tensor\n"
  "  -f, --fea             output only material data for FEA\n"
  "  -L, --license         print the license\n"
  "  -v, --version         show program's version number and exit\n"
  "  --log                 logging level debug,info,(default) warn,error,crit\n";

Options setup(int argc, char *argv[])
{
#ifndef NDEBUG
  info("argc = %d", argc);
  for (int32_t j = 0; j < argc; j++) {
    info("argv[%d] = “%s”", j, argv[j]);
  }
#endif
  Options rv = {0};
  int32_t choice;
  static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"info", no_argument, 0, 'i'},
    {"latex", no_argument, 0, 'l'},
    {"html", no_argument, 0, 'H'},
    {"eng", no_argument, 0, 'e'},
    {"mat", no_argument, 0, 'm'},
    {"fea", no_argument, 0, 'f'},
    {"license", no_argument, 0, 'L'},
    {"version", no_argument, 0, 'v'},
    {"log", required_argument, 0, 1000},
    {0,0,0,0}
  };
  logging_configure("lamprop-c", LOG_WARNING);
  while (1) {
    int32_t option_index = 0;
    choice = getopt_long(argc, argv, "hilHemfLv", long_options, &option_index);
    if (choice == -1) {
      break;
    }
    switch (choice) {
      case 'h':
        printf(help);
        exit(0);
        break;
      case 'i':
        rv.info = true;
        break;
      case 'l':
        if (rv.output==0) {
          rv.output = LATEX;
        }
        break;
      case 'H':
        if (rv.output==0) {
          rv.output = HTML;
        }
        break;
      case 'e':
        rv.eng = true;
        break;
      case 'm':
        rv.matrix = true;
        break;
      case 'f':
        rv.fea = true;
        break;
      case 'L':
        printf("lamprop-c version: %s\n", VERSION);
        printf(license);
        exit(0);
        break;
      case 'v':
        printf(VERSION "\n");
        exit(0);
        break;
      case 1000:
        if (strcasecmp(optarg, "debug")==0) {
          logging_configure(0, LOG_DEBUG);
        } else if (strcasecmp(optarg, "info")==0) {
          logging_configure(0, LOG_INFO);
        } else if (strcasecmp(optarg, "error")==0) {
          logging_configure(0, LOG_ERROR);
        } else if (strcasecmp(optarg, "crit")==0) {
          logging_configure(0, LOG_CRITICAL);
        }
        break;
    }
  }
  // Save updated values, skipping the executable name.
  rv.argc = argc - optind;
  rv.argv = argv + optind;
  // If none of the output options are given, set all of them
  if (rv.eng==false && rv.matrix == false && rv.fea == false) {
    rv.eng = true;
    rv.matrix = true;
    rv.fea = true;
  }
  return rv;
}
