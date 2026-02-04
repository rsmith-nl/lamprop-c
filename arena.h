// file: arena.h
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2023-04-23T22:07:59+0200
// Last modified: 2025-09-21T08:28:44+0200

// Inspired by: https://nullprogram.com/blog/2023/09/27/

#pragma once

#include <stdint.h>    // for uint8_t
#include <stddef.h>    // for size_t, ptrdiff_t, alignof
#include <stdalign.h>  // for alignof

typedef struct {
  uint8_t *begin;
  ptrdiff_t current_offset;
  ptrdiff_t length;
} Arena;

#ifdef __cplusplus
extern "C" {
#endif

// Create an arena of the given “length”. If “length” is 0, 1 MiB is used.
extern Arena arena_create(ptrdiff_t length);
// Return the remaining space in bytes in the arena.
extern ptrdiff_t arena_remaining(Arena *arena);
// Allocate from the arena. Best to use arena_new instead.
extern void *arena_alloc(Arena *arena, ptrdiff_t size, ptrdiff_t count,
                         ptrdiff_t align);
// Allocate “n” objects of type “t” from arena “a”.
#define arena_new(a, t, n) (t *)arena_alloc(a, sizeof(t), n, alignof(t))
// Destroy the arena and make it invalid.
extern void arena_destroy(Arena *arena);
// Empty out the arena.
extern void arena_empty(Arena *arena);

#ifdef __cplusplus
}
#endif
