//  file: arena.h
//  vim:fileencoding=utf-8:fdm=marker:ft=c
//
//  Copyright © 2023 R.F. Smith <rsmith@xs4all.nl>
//  SPDX-License-Identifier: MIT
//  Created: 2023-04-23T22:07:59+0200
//  Last modified: 2025-08-04T20:26:36+0200

#pragma once

#include <stdint.h>    // for uint8_t, int32_t
#include <stddef.h>    // for size_t, ptrdiff_t, alignof
#include <stdalign.h>  // for alignof

typedef struct {
  uint32_t magic;  // magic number to identify an arena
  uint8_t *begin;
  uint8_t *cur;
  uint8_t *guard;
} Arena;

#ifdef __cplusplus
extern "C" {
#endif

// Create an arena of the given *length*. If *length* is 0, 1 MiB is used.
extern Arena arena_create(ptrdiff_t length);
// Return the remaining space in the arena.
extern ptrdiff_t arena_remaining(Arena *arena);
// Allocate from the arena. Use arena_new instead.
extern void *arena_alloc(Arena *arena, ptrdiff_t size, ptrdiff_t count,
                         ptrdiff_t align);
// Allocate *n* objects of *type* from arena *a*.
#define arena_new(a, t, n) (t *)arena_alloc(a, sizeof(t), n, alignof(t))
// Destroy the arena and make it invalid.
extern void arena_destroy(Arena *arena);
// Empty out the arena.
extern void arena_empty(Arena *arena);

#ifdef __cplusplus
}
#endif
