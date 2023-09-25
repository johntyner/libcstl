#ifndef CSTL_BINTREE_H
#define CSTL_BINTREE_H

#include <stddef.h>

struct bintree_node {
    struct bintree_node * p, * l, * r;
};

typedef int (compar_t)(const void *, const void *);

struct bintree {
    struct bintree_node * root;
    size_t count;

    size_t off;
    compar_t * cmp;
};

#endif

