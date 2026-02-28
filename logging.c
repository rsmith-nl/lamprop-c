// file: logging.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2024-08-31 23:26:12 +0200
// Last modified: 2026-02-28T20:57:49+0100

#include "logging.h"

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

static char log_name[256] = {0};
static const char *prefix = "#%s %s: ";
static int log_level = LOG_WARNING;

void logging_configure(char *name, int level)
{
  assert(level>=0);
  if (name) {
    strncpy(log_name, name, 255);
  }
  if (level>0) {
    log_level = level;
  }
}

void debug(char *fmt, ...)
{
  if (log_level < LOG_DEBUG) {
    return;
  }
  va_list args = {0};
  assert(fmt);
  fprintf(stderr, prefix, log_name, "DEBUG");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fputs("\n", stderr);
  fflush(stderr);
  va_end(args);
}

void info(char *fmt, ...)
{
  if (log_level < LOG_INFO) {
    return;
  }
  va_list args = {0};
  assert(fmt);
  fprintf(stderr, prefix, log_name, "INFO");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fputs("\n", stderr);
  fflush(stderr);
  va_end(args);
}

void warning(char *fmt, ...)
{
  if (log_level < LOG_WARNING) {
    return;
  }
  va_list args = {0};
  assert(fmt);
  fprintf(stderr, prefix, log_name, "WARNING");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fputs("\n", stderr);
  fflush(stderr);
  va_end(args);
}

void error(char *fmt, ...)
{
  if (log_level < LOG_ERROR) {
    return;
  }
  va_list args = {0};
  assert(fmt);
  fprintf(stderr, prefix, log_name, "ERROR");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fputs("\n", stderr);
  fflush(stderr);
  va_end(args);
}

void critical(char *fmt, ...)
{
  if (log_level < LOG_CRITICAL) {
    return;
  }
  va_list args = {0};
  assert(fmt);
  fprintf(stderr, prefix, log_name, "CRITICAL");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fputs("\n", stderr);
  fflush(stderr);
  va_end(args);
  abort();
}
