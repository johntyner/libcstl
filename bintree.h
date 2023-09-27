#ifndef CSTL_BINTREE_H
#define CSTL_BINTREE_H

#include <stddef.h>

typedef int (compar_t)(const void *, const void *);

struct bintree_node {
    struct bintree_node * p, * l, * r;
};

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

void bintree_insert(struct bintree *, struct bintree_node *);
const void * bintree_find(const struct bintree *, const void *);

const struct bintree_node * __bintree_erase(
    struct bintree *, struct bintree_node *);
static inline void bintree_erase(struct bintree * const t,
                                 struct bintree_node * const n)
{
    __bintree_erase(t, n);
}

int __bintree_walk(const struct bintree_node *,
                   int (*)(const struct bintree_node *, void *), void *,
                   struct bintree_node ** (*)(struct bintree_node *),
                   struct bintree_node ** (*)(struct bintree_node *));

typedef enum {
    BINTREE_WALK_DIR_FWD,
    BINTREE_WALK_DIR_REV,
} bintree_walk_dir_t;
int bintree_walk(const struct bintree *,
                 int (*)(const void *, void *), void *,
                 bintree_walk_dir_t);

static inline struct bintree_node ** __bintree_left(
    struct bintree_node * const n)
{
    return &n->l;
}

static inline struct bintree_node ** __bintree_right(
    struct bintree_node * const n)
{
    return &n->r;
}

int bintree_cmp(const struct bintree *,
                const struct bintree_node *, const struct bintree_node *);

void __bintree_rotate(struct bintree *, struct bintree_node *,
                      struct bintree_node ** (*)(struct bintree_node *),
                      struct bintree_node ** (*)(struct bintree_node *));

static inline void __bintree_rotl(struct bintree * const bt,
                                  struct bintree_node * const bn)
{
    __bintree_rotate(bt, bn, __bintree_left, __bintree_right);
}

static inline void __bintree_rotr(struct bintree * const bt,
                                  struct bintree_node * const bn)
{
    __bintree_rotate(bt, bn, __bintree_right, __bintree_left);
}

void bintree_height(const struct bintree *, size_t *, size_t *);

#endif
