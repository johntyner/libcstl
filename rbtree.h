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

static inline size_t rbtree_size(const struct rbtree * const t)
{
    return bintree_size(&t->t);
}

#endif
