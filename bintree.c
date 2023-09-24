#include "bintree.h"

void bintree_init(struct bintree * const bt,
                  bintree_node_cmp_t * const cmp)
{
    bt->root  = NULL;
    bt->count = 0;

    bt->cmp   = cmp;
}

void __bintree_insert(struct bintree_node * bp,
                      struct bintree_node * const bn,
                      bintree_node_cmp_t * const cmp)
{
    struct bintree_node ** bc = &bp;

    while (*bc != NULL) {
        bp = *bc;

        if (cmp(bp, bn) < 0) {
            bc = &bp->l;
        } else {
            bc = &bp->r;
        }
    }

    *bc = bn;

    bn->p = bp;
    bn->l = NULL;
    bn->r = NULL;
}

void bintree_insert(struct bintree * const bt, struct bintree_node * const bn)
{
    if (bt->root != NULL) {
        __bintree_insert(bt->root, bn, bt->cmp);
    } else {
        bt->root = bn;

        bn->p = NULL;
        bn->l = NULL;
        bn->r = NULL;
    }
}

#define BINTREE_SLIDE(BN, DIR)                  \
    do {                                        \
        while (BN->DIR != NULL) {               \
            BN = BN->DIR;                       \
        }                                       \
    } while (0)

/* same as bintree_first(), cannot return NULL */
static struct bintree_node * __bintree_min(struct bintree_node * bn)
{
    BINTREE_SLIDE(bn, l);
    return bn;
}

/* same as bintree_last(), cannot return NULL */
static struct bintree_node * __bintree_max(struct bintree_node * bn)
{
    BINTREE_SLIDE(bn, r);
    return bn;
}

#define BINTREE_ADJACENT(BN, DIR, MINMAX)               \
    do {                                                \
        if (BN->DIR != NULL) {                          \
            BN = MINMAX(BN->DIR);                       \
        } else {                                        \
            while (BN->p != NULL && BN->p->DIR == BN) { \
                BN = BN->p;                             \
            }                                           \
            BN = BN->p;                                 \
        }                                               \
    } while (0)

/* could return NULL */
static struct bintree_node * __bintree_next(struct bintree_node * bn)
{
    BINTREE_ADJACENT(bn, r, __bintree_min);
    return bn;
}

/* could return NULL */
static struct bintree_node * __bintree_prev(struct bintree_node * bn)
{
    BINTREE_ADJACENT(bn, l, __bintree_max);
    return bn;
}

void bintree_erase(struct bintree * const bt,
                   struct bintree_node * const bn)
{
    struct bintree_node * x, * y;

    if (bn->l != NULL && bn->r != NULL) {
        y = __bintree_next(bn);
    } else {
        y = bn;
    }

    if (y->l != NULL) {
        x = y->l;
    } else {
        x = y->r;
    }

    if (x != NULL) {
        x->p = y->p;
    }

    if (y->p == NULL) {
        bt->root = x;
    } else if (y == y->p->l) {
        y->p->l = x;
    } else {
        y->p->r = x;
    }

    if (y != bn) {
        if (bn->p == NULL) {
            bt->root = y;
        } else if (bn == bn->p->l) {
            bn->p->l = y;
        } else {
            bn->p->r = y;
        }

        if (bn->l != NULL) {
            bn->l->p = y;
        }
        if (bn->r != NULL) {
            bn->r->p = y;
        }

        *y = *bn;
    }
}
