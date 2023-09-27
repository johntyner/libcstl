#ifndef CSTL_RBTREE_H
#define CSTL_RBTREE_H

#include "bintree.h"

typedef enum {
    RBTREE_COLOR_R,
    RBTREE_COLOR_B,
} rbtree_color_t;

struct rbtree_node {
    struct bintree_node n;
    rbtree_color_t c;
};

struct rbtree {
    struct bintree t;

    size_t off;
};

void rbtree_init(struct rbtree *, compar_t *, size_t);
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

#endif
