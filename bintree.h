#ifndef CSTL_BINTREE_H
#define CSTL_BINTREE_H

#include <stddef.h>

#include "common.h"

struct bintree_node {
    struct bintree_node * p, * l, * r;
};

struct bintree {
    struct bintree_node * root;
    size_t size;

    size_t off;
    cstl_compare_func_t * cmp;
};

static inline void bintree_init(struct bintree * const bt,
                                cstl_compare_func_t * const cmp,
                                const size_t off)
{
    bt->root    = NULL;
    bt->size    = 0;

    bt->off     = off;
    bt->cmp     = cmp;
}
#define BINTREE_INIT(BT, TYPE, MEMB, CMP)       \
    bintree_init(BT, CMP, offsetof(TYPE, MEMB))

static inline size_t bintree_size(const struct bintree * const bt)
{
    return bt->size;
}

void bintree_insert(struct bintree *, void *);
const void * bintree_find(const struct bintree *, const void *);

const struct bintree_node * __bintree_erase(
    struct bintree *, struct bintree_node *);
void * bintree_erase(struct bintree *, const void *);

void bintree_clear(struct bintree *, cstl_clear_func_t *);

void bintree_swap(struct bintree *, struct bintree *);

typedef struct bintree_node ** (bintree_child_func_t)(struct bintree_node *);

typedef enum {
    BINTREE_VISIT_ORDER_PRE,
    BINTREE_VISIT_ORDER_MID,
    BINTREE_VISIT_ORDER_POST,
    BINTREE_VISIT_ORDER_LEAF,
} bintree_visit_order_t;

int __bintree_foreach(const struct bintree_node *,
                      int (*)(const struct bintree_node *,
                              bintree_visit_order_t,
                              void *), void *,
                      bintree_child_func_t *,
                      bintree_child_func_t *);

typedef enum {
    BINTREE_WALK_DIR_FWD,
    BINTREE_WALK_DIR_REV,
} bintree_foreach_dir_t;

int bintree_foreach(const struct bintree *,
                    cstl_const_visit_func_t *, void *,
                    bintree_foreach_dir_t);

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
                      bintree_child_func_t *,
                      bintree_child_func_t *);

typedef void (bintree_rotate_func_t)(struct bintree *, struct bintree_node *);

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
