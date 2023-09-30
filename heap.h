#ifndef CSTL_HEAP_H
#define CSTL_HEAP_H

#include "bintree.h"

static inline void heap_init(struct bintree * const h,
                             compar_t * const cmp, const size_t off)
{
    bintree_init(h, cmp, off);
}
#define HEAP_INIT(H, TYPE, MEMB, CMP)           \
    heap_init(H, CMP, offsetof(TYPE, MEMB))

void heap_push(struct bintree *, void *);

const void * heap_get(const struct bintree *);
void * heap_pop(struct bintree *);

static inline void heap_swap(struct bintree * const a, struct bintree * const b)
{
    bintree_swap(a, b);
}

static inline void heap_clear(struct bintree * const h,
                              void (* const clr)(void *))
{
    bintree_clear(h, clr);
}

#endif
