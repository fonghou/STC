#ifndef ARENA_H
#define ARENA_H

/** Credit:
    https://nullprogram.com/blog/2023/09/27/
    https://lists.sr.ht/~skeeto/public-inbox/%3C20231015233305.sssrgorhqu2qo5jr%40nullprogram.com%3E
    https://nullprogram.com/blog/2023/10/05/
*/

#include <assert.h>
#include <memory.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef ptrdiff_t ssize;
typedef unsigned char byte;

typedef struct Arena Arena;
struct Arena {
  byte **start;
  byte *stop;
  void **oomjmp;
  Arena *persist;
};

enum {
  SOFTFAIL = 1 << 0,
  NOINIT = 1 << 1,
};

#define ref(x)    const x[static 1]
#define mutref(x) x[static 1]

/** Usage:

  ssize cap = 1 << 20;
  void *heap = malloc(cap);
  byte *p = heap;
  Arena global = newarena(&p, cap);

  if (ARENA_OOM(&global)) {
    abort();
  }

  Arena scratch = getscratch(&global);

  thing *x = New(&global, thing);
  thing *y = New(&scratch, thing);
  thing *z = helper(scratch);

  free(buffer);

*/
static inline Arena newarena(byte **mem, ssize size) {
  Arena a = {0};
  a.start = mem;
  a.stop = *mem ? *mem + size : 0;
  return a;
}

#define New(...)                       ARENA_NEWX(__VA_ARGS__, ARENA_NEW4, ARENA_NEW3, ARENA_NEW2)(__VA_ARGS__)
#define ARENA_NEWX(a, b, c, d, e, ...) e
#define ARENA_NEW2(a, t)               (t *)arena_alloc(a, sizeof(t), alignof(t), 1, 0)
#define ARENA_NEW3(a, t, n)            (t *)arena_alloc(a, sizeof(t), alignof(t), n, 0)
#define ARENA_NEW4(a, t, n, z)         (t *)arena_alloc(a, sizeof(t), alignof(t), n, z)

#define ArenaOOM(A)                              \
  ({                                             \
    Arena *a_ = (A);                             \
    a_->oomjmp = New(a_, void *, 5, SOFTFAIL);   \
    !a_->oomjmp || __builtin_setjmp(a_->oomjmp); \
  })

#define Push(S, A)                                               \
  ({                                                             \
    typeof(S) s_ = (S);                                          \
    if (s_->len >= s_->cap) {                                    \
      slice_grow(s_, sizeof(*s_->data), sizeof(*s_->data), (A)); \
    }                                                            \
    s_->data + s_->len++;                                        \
  })

// clang-format off
#ifdef NDEBUG
#  define LogArena(A)
#else
#  define LogArena(A)                                                   \
     fprintf(stderr, "%s:%d: Arena " #A "\tbeg=%ld end=%ld diff=%ld\n", \
             __FILE__,                                                  \
             __LINE__,                                                  \
            (uintptr_t)(*(A).start),                                  \
            (uintptr_t)(A).stop,                                     \
            (ssize)((A).stop - (*(A).start)))
#endif

#ifdef NDEBUG
#  define assert(c) while (!(c)) __builtin_unreachable()
#else
#  include <assert.h>
#endif

__attribute((malloc, alloc_size(2, 4), alloc_align(3))) static inline
byte* arena_alloc(Arena *a, ssize size, ssize align, ssize count, unsigned flags) {
  // clang-format on
  byte *r = 0;
  // sync [2]
  if (a->persist) {
    byte *start = *a->persist->start;
    if (*a->start < a->stop) {
      a->stop = start < a->stop ? start : a->stop;
      if (*a->start > a->stop) goto OOM;
    } else {
      a->stop = a->stop < start ? start : a->stop;
      if (*a->start < a->stop) goto OOM;
    }
  }

  if (*a->start < a->stop) {
    ssize avail = a->stop - *a->start;
    ssize padding = -(uintptr_t)*a->start & (align - 1);
    if (count > (avail - padding) / size) goto OOM;
    r = *a->start + padding;
    *a->start += padding + size * count;
  } else {
    ssize avail = *a->start - a->stop;
    ssize padding = +(uintptr_t)*a->start & (align - 1);
    if (count > (avail - padding) / size) goto OOM;
    *a->start -= padding + size * count;
    r = *a->start;
  }

  return flags & NOINIT ? r : (byte *)memset(r, 0, size * count);

OOM:
  if (flags & SOFTFAIL || !a->oomjmp) return NULL;
#ifndef OOM
  assert(a->oomjmp);
  __builtin_longjmp(a->oomjmp, 1);
#else
  assert(!OOM);
#endif
}

static inline void slice_grow(void *slice, ssize size, ssize align, Arena *a) {
  struct {
    void *data;
    ssize len;
    ssize cap;
  } replica;
  memcpy(&replica, slice, sizeof(replica));

  assert(replica.len >= 0);
  assert(replica.cap >= 0);
  assert(replica.len <= replica.cap);

  if (!replica.data) {
    replica.cap = 1;
    replica.data = arena_alloc(a, 2 * size, align, replica.cap, 0);
  } else if ((*a->start < a->stop &&
              ((uintptr_t)*a->start - size * replica.cap == (uintptr_t)replica.data))) {
    arena_alloc(a, size, 1, replica.cap, 0);
  } else {
    void *data = arena_alloc(a, 2 * size, align, replica.cap, 0);
    void *src = replica.data;
    void *dest = data;
    ssize len = size * replica.len;
    memcpy(dest, src, len);
    replica.data = data;
  }

  replica.cap *= 2;
  memcpy(slice, &replica, sizeof(replica));
}

static inline bool isscratch(Arena *a) {
  return *a->start > a->stop;
}

static inline Arena getscratch(Arena *a) {
  if (isscratch(a)) return *a;  // guard [2]

  Arena scratch = {0};
  scratch.start = &a->stop;
  scratch.stop = *a->start;
  scratch.oomjmp = a->oomjmp;
  scratch.persist = a;
  return scratch;
}

#endif  // ARENA_H
