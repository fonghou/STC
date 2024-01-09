#ifndef STCARENA_H
#define STCARENA_H

#include "mem/arena.h"

static inline byte *arena_realloc(Arena *a, void *src, ssize old_sz, ssize sz) {
  void *dest = arena_alloc(a, 1, MAX_ALIGN, sz, NOINIT);
  assert(dest < src || dest - old_sz >= src);
  assert(src < dest || src - old_sz >= dest);
  return src ? memcpy(dest, src, old_sz) : dest;
}

#define arena_malloc(sz)             arena_alloc(c_extend()->arena, 1, MAX_ALIGN, (sz), NOINIT)
#define arena_calloc(n, sz)          arena_alloc(c_extend()->arena, sz, sz, n, 0)
#define arena_realloc(p, old_sz, sz) arena_realloc(c_extend()->arena, p, (old_sz), (sz))
#define arena_free(p, sz)            // p ? memset(p, 0x0, sz) : 0;

#endif

#ifdef i_type

#define i_allocator arena
#define i_no_clone
#define i_extend Arena *arena;

#include "stc/extend.h"

#undef i_allocator
#undef i_no_clone
#undef i_extend

#endif
