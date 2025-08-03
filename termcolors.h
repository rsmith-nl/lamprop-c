// file: termcolors.h
// vim:fileencoding=utf-8:ft=cpp:tabstop=2
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// Created: 2025-01-19 18:39:29 +0100
// Last modified: 2025-01-19T18:41:35+0100

#pragma once

#define BOLD_WHITE "\033[1;37m"
#define CYAN "\033[0;36m"
#define GREEN "\033[0;32m"
#define PURPLE "\033[0;35m"
#define BOLD_RED "\033[1;31m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define BOLD_YELLOW "\033[1;33m"
#define RESET "\033[0m"  // No Color

// Usage e.g: printf(PURPLE"WARNING:"RESET" not enough memory!\n");
