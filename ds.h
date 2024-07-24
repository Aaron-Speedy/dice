#ifndef DS_H
#define DS_H

#ifndef CUSTOM_ASSERT
#include <assert.h>
#define CUSTOM_ASSERT(c) assert(c)
#endif

#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <time.h>

#define List(T) struct { T *buf; size_t cap, count; }

typedef struct {
  char *buf;
  size_t cap, pos;
} Arena;

Arena arena_init(size_t cap);
void *arena_alloc(Arena *al, size_t len);
void arena_free_all(Arena *al);

#ifdef DS_IMPL
#undef DS_IMPL

#define arrlen(...) (size_t)(sizeof(__VA_ARGS__) / sizeof(*__VA_ARGS__))

#define da_push(xs, x) \
  do { \
    if ((xs)->count >= (xs)->cap) { \
      if ((xs)->cap == 0) (xs)->cap = 256; \
      else (xs)->cap *= 2; \
      (xs)->items = realloc((xs)->items, (xs)->cap*sizeof(*(xs)->items)); \
    } \
 \
    (xs)->items[(xs)->count++] = (x); \
  } while (0)

#define da_pop(xs) \
  do { \
    assert ((xs)->count > 0); \
    (xs)->count -= 1; \
  } while (0)

#define da_last(xs) (assert((xs)->count > 0), (xs)->items[(xs)->count - 1])

#define da_init(xs) \
do { \
  (xs)->items = malloc((xs)->cap * sizeof((xs)->items[0])); \
} while (0);

Arena arena_init(size_t cap) {
  CUSTOM_ASSERT(cap > 0);

  Arena al = { .cap = cap, };
  al.buf = malloc(al.cap);

  return al;
}

void *arena_alloc(Arena *al, size_t len) {
  CUSTOM_ASSERT(al->pos + len <= al->cap && "Not enough memory in arena");
  CUSTOM_ASSERT(len > 0);

  void *mem = &al->buf[al->pos];
  al->pos += len;

  return mem;
}

void arena_free_all(Arena *al) {
  al->pos = 0;
}

#endif

#endif
