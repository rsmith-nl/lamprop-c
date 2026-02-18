// file: arena.c
// vim:fileencoding=utf-8:ft=c:tabstop=2
// This is free and unencumbered software released into the public domain.
//
// Author: R.F. Smith <rsmith@xs4all.nl>
// SPDX-License-Identifier: Unlicense
// Created: 2023-04-23T22:08:02+0200
// Last modified: 2026-02-18T19:15:43+0100

#include "arena.h"
#include "logging.h"
#include <assert.h>
#include <stdio.h>      // for printf
#include <stdint.h>     // for uintptr_t
#include <stddef.h>     // for ptrdiff_t
#include <string.h>     // for memset
#ifdef _WIN32
#include <memoryapi.h>
#define MAP_FAILED ((void *)-1)
#else
#include <sys/mman.h>   // for mmap, munmap
#endif

Arena arena_create(ptrdiff_t length)
{
  Arena arena = {0};
  // Default length 1 MiB.
  if (length <= 0) {
    length = 1048576;
  }
#ifdef _WIN32
  arena.begin = VirtualAlloc(0, length, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
  if (arena.begin == 0) {
    arena.begin = MAP_FAILED;
  }
#else
  arena.begin = mmap(0, length, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
#endif
  if (arena.begin == MAP_FAILED) {
    error("arena allocation of size %td failed\n", length);
  }
  arena.current_offset = 0;
  arena.length = length;
  return arena;
}

ptrdiff_t arena_remaining(Arena *arena)
{
  assert(arena!=0);
  return arena->length - arena->current_offset;
}

void *arena_alloc(Arena *arena, ptrdiff_t size, ptrdiff_t count, ptrdiff_t align)
{
  assert(arena!=0);
  assert(size>0);
  assert(count>0);
  ptrdiff_t padding = -arena->current_offset & (align - 1);
  ptrdiff_t remaining = arena->length - arena->current_offset - padding;
  if (count > remaining/size) {
    error("arena %p exhausted; %td items of %td bytes requested, %td available\n",
          (void *)arena, count, size, remaining/size);
  }
  void *rv = arena->begin + arena->current_offset + padding;
  arena->current_offset += padding + count * size;
  return memset(rv, 0, count * size);
}

void arena_destroy(Arena *arena)
{
  assert(arena!=0);
  int rv;
#ifdef _WIN32
  rv = VirtualFree(arena->begin, 0, MEM_RELEASE);
  if (rv == 0) {
    rv = -1;
  }
#else
  rv = munmap(arena->begin, arena->length);
#endif
  if (rv == -1) {
    error("destroying arena %p failed\n", (void *)arena);
  }
  Arena empty = {0};
  *arena = empty;
}

void arena_empty(Arena *arena)
{
  assert(arena!=0);
  // Clear all the used memory.
  memset(arena->begin, 0, arena->current_offset);
  // Reset the use counter.
  arena->current_offset = 0;
}
