#ifndef CSTL_BINTREE_H
#define CSTL_BINTREE_H

#include <stddef.h>
#include <stdint.h>

typedef int (compar_t)(const void *, const void *);

struct bintree_node {
    struct bintree_node * p, * l, * r;
};

static inline const void * __bintree_element(
    const struct bintree_node * const bn, const size_t off)
{
    return (void *)((uintptr_t)bn - off);
}

static int __bintree_cmp(const struct bintree_node * const a,
                         const struct bintree_node * const b,
                         compar_t * const cmp, const size_t off)
{
    return cmp(__bintree_element(a, off), __bintree_element(b, off));
}

struct bintree {
    struct bintree_node * root;
    size_t count;

    size_t off;
    compar_t * cmp;
};

static inline void bintree_init(struct bintree * const bt,
                                compar_t * const cmp, const size_t off)
{
    bt->root  = NULL;
    bt->count = 0;

    bt->off   = off;
    bt->cmp   = cmp;
}

static inline size_t bintree_size(const struct bintree * const bt)
{
    return bt->count;
}

static inline const void * bintree_element(
    const struct bintree * const bt, const struct bintree_node * const bn)
{
    return __bintree_element(bn, bt->off);
}
#define BINTREE_ELEMENT(BT, BN, TYPE)   ((const TYPE *)bintree_element(BT, BN))

void bintree_init(struct bintree *, compar_t *, size_t);

void bintree_insert(struct bintree *, struct bintree_node *);
const void * bintree_find(const struct bintree *, const void *);

struct bintree_node * __bintree_erase(struct bintree *, struct bintree_node *);
static inline void bintree_erase(
    struct bintree * const t, struct bintree_node * const n)
{
    __bintree_erase(t, n);
}

const void * bintree_first(const struct bintree *);
const void * bintree_next(const struct bintree *, const struct bintree_node *);
const void * bintree_prev(const struct bintree *, const struct bintree_node *);
const void * bintree_last(const struct bintree *);

int __bintree_walk(const struct bintree_node *,
                   int (*)(const struct bintree_node *, void *), void *);
int bintree_walk(const struct bintree *,
                 int (*)(const void *, void *), void *);

void __bintree_rotate_l(struct bintree *, struct bintree_node *);
void __bintree_rotate_r(struct bintree *, struct bintree_node *);

void bintree_height(const struct bintree *, size_t *, size_t *);

#endif
