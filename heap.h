#ifndef CSTL_HEAP_H
#define CSTL_HEAP_H

#include "bintree.h"
#include "common.h"

struct heap_node
{
    struct bintree_node bn;
};

struct heap
{
    struct bintree bt;
};

static inline void heap_init(struct heap * const h,
                             cstl_compare_func_t * const cmp, const size_t off)
{
    bintree_init(&h->bt, cmp, off + offsetof(struct heap_node, bn));
}
#define HEAP_INIT(H, TYPE, MEMB, CMP)           \
    heap_init(H, CMP, offsetof(TYPE, MEMB))

static inline size_t heap_size(const struct heap * const h)
{
    return bintree_size(&h->bt);
}

void heap_push(struct heap *, void *);

const void * heap_get(const struct heap *);
void * heap_pop(struct heap *);

static inline void heap_swap(struct heap * const a, struct heap * const b)
{
    bintree_swap(&a->bt, &b->bt);
}

static inline void heap_clear(struct heap * const h,
                              cstl_clear_func_t * const clr)
{
    bintree_clear(&h->bt, clr);
}

#endif
