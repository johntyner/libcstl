#ifndef CSTL_HEAP_H
#define CSTL_HEAP_H

#include "bintree.h"

static inline void heap_init(struct bintree * const h,
                             compar_t * const cmp, const size_t off)
{
    bintree_init(h, cmp, off);
}

void heap_push(struct bintree *, void *);

const void * heap_get(const struct bintree *);
void * heap_pop(struct bintree *);

#endif
