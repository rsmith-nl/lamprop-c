// file: logging.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2024-08-31 23:25:54 +0200
// Last modified: 2026-02-22T00:09:03+0100

#pragma once

enum {
  LOG_NOTSET = 0,
  LOG_CRITICAL = 10,
  LOG_ERROR = 20,
  LOG_WARNING = 30,
  LOG_INFO = 40,
  LOG_DEBUG = 50
};

#ifdef __cplusplus
extern "C" {
#endif

// Call this function before any of the other logging calls below.
// The “name” is prepended to each log message.
// Every message with a “level” equal or lower than the given value is printed.
// If “level” is 0, the logging level is set to LOG_WARNING.
extern void logging_configure(char *name, int level);

// Debug message. For developer infomation.
extern void debug(char *fmt, ...);

// Informational message.
extern void info(char *fmt, ...);

// An indication that something unexpected happened.
extern void warning(char *fmt, ...);

// Due to a more serious problem, the software has not been able to perform some function.
extern void error(char *fmt, ...);

// A problem so serious that the program cannot continue and will be aborted.
extern void critical(char *fmt, ...);

#ifdef __cplusplus
}
#endif
