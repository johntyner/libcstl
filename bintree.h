#ifndef CSTL_BINTREE_H
#define CSTL_BINTREE_H

#include <stddef.h>

struct bintree_node {
    struct bintree_node * p, * l, * r;
};

typedef int(bintree_node_cmp_t)(const struct bintree_node *,
                                const struct bintree_node *);

struct bintree {
    struct bintree_node * root;
    size_t count;

    bintree_node_cmp_t * cmp;
};

#endif

