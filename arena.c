//  file: arena.c
//  vim:fileencoding=utf-8:fdm=marker:ft=c
//
//  Copyright © 2023 R.F. Smith <rsmith@xs4all.nl>
//  SPDX-License-magicifier: MIT
//  Created: 2023-04-23T22:08:02+0200
//  Last modified: 2025-04-13T15:04:18+0200

#include "arena.h"
#include <stdio.h>     // for printf
#include <stdlib.h>    // for abort
#include <string.h>    // for memset
#include <sys/mman.h>  // for mmap, munmap

#undef error
#define error(...)                                            \
  fprintf(stderr, "ERROR %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__);                               \
  abort()

#ifdef NDEBUG
#define debug(...)  ((void)0)
#define info(...) ((void)0)
#else
#define debug(...) \
  fprintf(stderr, "DEBUG %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n")
#define info(...)                                            \
  fprintf(stderr, "INFO %s, line %i: ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n")
#endif  // NDEBUG

// In [9]: '0x' + ''.join([hex(ord(j))[2:] for j in "AREN"])
// Out[9]: '0x4152454e'
#define ARENA_MAGIC 0x4152454e

Arena arena_create(ptrdiff_t length)
{
  Arena arena = {0};
  // Default length 1 MiB.
  if (length <= 0) {
    length = 1048576;
  }
  arena.magic = ARENA_MAGIC;
  arena.begin =
    mmap(0, length, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
  if (arena.begin == MAP_FAILED) {
    error("arena allocation of size %td failed\n", length);
  }
  info("arena; %td bytes allocated", length);
  arena.cur = arena.begin;
  // Do *not* write to the location pointed to by guard!
  // it is outside the allocated area.
  arena.guard = arena.begin + length;
  return arena;
}

ptrdiff_t arena_remaining(Arena *arena)
{
  if (arena == 0) {
    error("null pointer used for arena\n");
  }
  if (arena->magic != ARENA_MAGIC) {
    error("invalid arena %p; magic %d\n", (void*)arena, arena->magic);
  }
  ptrdiff_t remaining = arena->guard - arena->cur;
  return remaining;
}

void *arena_alloc(Arena *arena, ptrdiff_t size, ptrdiff_t count, ptrdiff_t align)
{
  if (arena == 0) {
    error("null pointer used for arena\n");
  }
  if (arena->magic != ARENA_MAGIC) {
    error("invalid arena %p; magic %d\n", (void *)arena, arena->magic);
  }
  ptrdiff_t padding = -(uintptr_t)arena->cur & (align - 1);
  ptrdiff_t remaining = arena->guard - arena->cur - padding;
  if (count > remaining/size) {
    error("arena %p exhausted; %td items of %td bytes requested, %td available\n",
          (void *)arena, count, size, remaining/size);
  }
  void *rv = arena->cur + padding;
  arena->cur += padding + count * size;
  return memset(rv, 0, count * size);
}

void arena_destroy(Arena *arena)
{
  if (arena == 0) {
    error("null pointer used for arena\n");
  }
  if (arena->magic != ARENA_MAGIC) {
    error("invalid arena %p; magic %d\n", (void *)arena, arena->magic);
  }
  int rv = munmap(arena->begin, arena->guard - arena->begin);
  if (rv == -1) {
    error("destroying arena %p failed\n", (void *)arena);
  }
  Arena empty = {0};
  *arena = empty;
}
