#ifndef CSTL_RBTREE_H
#define CSTL_RBTREE_H

#include "bintree.h"

typedef enum {
    RBTREE_COLOR_R,
    RBTREE_COLOR_B,
} rbtree_color_t;

struct rbtree_node {
    rbtree_color_t c;
    struct bintree_node n;
};

struct rbtree {
    struct bintree t;

    size_t off;
};

static inline void rbtree_init(struct rbtree * const t,
                               cstl_compare_func_t * const cmp,
                               const size_t off)
{
    bintree_init(&t->t, cmp, off + offsetof(struct rbtree_node, n));
    t->off = off;
}
#define RBTREE_INIT(T, TYPE, MEMB, CMP)         \
    rbtree_init(T, CMP, offsetof(TYPE, MEMB))

static inline size_t rbtree_size(const struct rbtree * const t)
{
    return bintree_size(&t->t);
}

void rbtree_insert(struct rbtree *, void *);

static inline const void * rbtree_find(
    const struct rbtree * const t, const void * const n)
{
    return bintree_find(&t->t, n);
}

void * rbtree_erase(struct rbtree *, const void *);

static inline void rbtree_swap(struct rbtree * const a, struct rbtree * const b)
{
    size_t t;
    bintree_swap(&a->t, &b->t);
    cstl_swap(&a->off, &b->off, &t, sizeof(t));
}

static inline void rbtree_clear(struct rbtree * const t,
                                cstl_clear_func_t * const clr)
{
    bintree_clear(&t->t, clr);
}

#endif
