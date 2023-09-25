#include "bintree.h"

#include <stdint.h>
#include <stdlib.h>

void bintree_init(struct bintree * const bt,
                  compar_t * const cmp, const size_t off)
{
    bt->root  = NULL;
    bt->count = 0;

    bt->off   = off;
    bt->cmp   = cmp;
}

size_t bintree_size(const struct bintree * const bt)
{
    return bt->count;
}

static const void * bintree_element(const struct bintree_node * const bn,
                                    const size_t off)
{
    return (void *)((uintptr_t)bn - off);
}

static int bintree_cmp(const struct bintree_node * const a,
                       const struct bintree_node * const b,
                       compar_t * const cmp, const size_t off)
{
    return cmp(bintree_element(a, off), bintree_element(b, off));
}

static struct bintree_node * __bintree_find(
    struct bintree_node * rt,
    const struct bintree_node * const bn,
    compar_t * const cmp, const size_t off)
{
    while (rt != NULL) {
        const int eq = bintree_cmp(bn, rt, cmp, off);

        if (eq < 0) {
            rt = rt->l;
        } else if (eq > 0) {
            rt = rt->r;
        } else {
            break;
        }
    }

    return rt;
}

static void __bintree_insert(struct bintree_node * bp,
                             struct bintree_node * const bn,
                             compar_t * const cmp, const size_t off)
{
    struct bintree_node ** bc = &bp;

    while (*bc != NULL) {
        bp = *bc;

        if (bintree_cmp(bn, bp, cmp, off) < 0) {
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
        __bintree_insert(bt->root, bn, bt->cmp, bt->off);
    } else {
        bt->root = bn;

        bn->p = NULL;
        bn->l = NULL;
        bn->r = NULL;
    }

    bt->count++;
}

#define BINTREE_SLIDE(BN, DIR)                  \
    do {                                        \
        while (BN->DIR != NULL) {               \
            BN = BN->DIR;                       \
        }                                       \
    } while (0)

/* same as bintree_first(), but cannot return NULL */
static struct bintree_node * __bintree_min(struct bintree_node * bn)
{
    BINTREE_SLIDE(bn, l);
    return bn;
}

/* same as bintree_last(), but cannot return NULL */
static struct bintree_node * __bintree_max(struct bintree_node * bn)
{
    BINTREE_SLIDE(bn, r);
    return bn;
}

const void * bintree_first(const struct bintree * const bt)
{
    if (bt->root == NULL) {
        return NULL;
    }

    return bintree_element(__bintree_min(bt->root), bt->off);
}

const void * bintree_last(const struct bintree * const bt)
{
    if (bt->root == NULL) {
        return NULL;
    }

    return bintree_element(__bintree_max(bt->root), bt->off);
}

#define BINTREE_NEXT(BN, DIR, MINMAX)                   \
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
    BINTREE_NEXT(bn, r, __bintree_min);
    return bn;
}

/* could return NULL */
static struct bintree_node * __bintree_prev(struct bintree_node * bn)
{
    BINTREE_NEXT(bn, l, __bintree_max);
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

    bt->count--;
}

static size_t __bintree_height(struct bintree_node * bn)
{
    size_t h;

    for (h = 0; bn != NULL; h++, bn = bn->p)
        ;

    return h;
}

void bintree_height(const struct bintree * const bt,
                    size_t * const min, size_t * const max)
{
    if (bt->root == NULL) {
        *min = 0;
        *max = 0;
    } else {
        struct bintree_node * bn;

        *min = SIZE_MAX;
        *max = 0;

        for (bn = __bintree_min(bt->root);
             bn != NULL;
             bn = __bintree_next(bn)) {
            if (bn->l == NULL && bn->r == NULL) {
                const size_t h = __bintree_height(bn);

                if (h < *min) {
                    *min = h;
                }

                if (h > *max) {
                    *max = h;
                }
            }
        }
    }
}

#ifdef __test__
#include <check.h>

struct integer {
    struct bintree_node bn;
    int v;
};

static int cmp_integer(const void * const a, const void * const b)
{
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

START_TEST(init)
{
    struct bintree bt;

    bintree_init(&bt, cmp_integer, offsetof(struct integer, bn));
}
END_TEST

static void __test__bintree_fill(struct bintree * const bt, const size_t n)
{
    size_t sz;

    while ((sz = bintree_size(bt)) < n) {
        struct integer * const in = malloc(sizeof(*in));
        struct bintree_node * bn;

        in->v = rand() % n;
        bn = __bintree_find(bt->root, &in->bn, bt->cmp, bt->off);
        if (bn == NULL) {
            bintree_insert(bt, &in->bn);
            ck_assert_uint_eq(sz + 1, bintree_size(bt));
        } else {
            free(in);
        }
    }
}

static void __test__bintree_drain(struct bintree * const bt)
{
    size_t sz;

    while ((sz = bintree_size(bt)) > 0) {
        struct bintree_node * bn = bt->root;

        bintree_erase(bt, bn);
        free((void *)bintree_element(bn, bt->off));

        ck_assert_uint_eq(sz - 1, bintree_size(bt));
    }

    ck_assert_ptr_null(bt->root);
    ck_assert_uint_eq(bintree_size(bt), 0);
}

START_TEST(fill)
{
    static const size_t n = 100;

    struct bintree bt;

    bintree_init(&bt, cmp_integer, offsetof(struct integer, bn));
    __test__bintree_fill(&bt, n);
    __test__bintree_drain(&bt);
}
END_TEST

START_TEST(walk_fwd)
{
    static const size_t n = 100;

    struct bintree bt;
    struct bintree_node * bn;
    unsigned int i;

    bintree_init(&bt, cmp_integer, offsetof(struct integer, bn));
    __test__bintree_fill(&bt, n);

    for (i = 0, bn = __bintree_min(bt.root);
         i < n;
         i++, bn = __bintree_next(bn)) {
        const struct integer * const in = bintree_element(bn, bt.off);
        ck_assert_uint_eq(i, in->v);
    }

    __test__bintree_drain(&bt);
}
END_TEST

START_TEST(walk_bck)
{
    static const size_t n = 100;

    struct bintree bt;
    struct bintree_node * bn;
    unsigned int i;

    bintree_init(&bt, cmp_integer, offsetof(struct integer, bn));
    __test__bintree_fill(&bt, n);

    for (i = 0, bn = __bintree_max(bt.root);
         i < n;
         i++, bn = __bintree_prev(bn)) {
        const struct integer * const in = bintree_element(bn, bt.off);
        ck_assert_uint_eq(n - i - 1, in->v);
    }

    __test__bintree_drain(&bt);
}
END_TEST

START_TEST(empty)
{
    static const size_t n = 100;

    struct bintree bt;

    bintree_init(&bt, cmp_integer, offsetof(struct integer, bn));
    __test__bintree_fill(&bt, n);

    struct bintree_node * bn;
    size_t sz;

    while ((sz = bintree_size(&bt)) > 0) {
        struct integer in;

        in.v = rand() % n;
        bn = __bintree_find(bt.root, &in.bn, bt.cmp, bt.off);

        if (bn != NULL) {
            bintree_erase(&bt, bn);
            free((void *)bintree_element(bn, bt.off));
            ck_assert_uint_eq(sz - 1, bintree_size(&bt));
        }
    }

    ck_assert_ptr_null(bt.root);
    ck_assert_uint_eq(bt.count, 0);
}
END_TEST

Suite * bintree_suite(void)
{
    Suite * const s = suite_create("bintree");

    TCase * tc;

    tc = tcase_create("bintree");
    tcase_add_test(tc, init);
    tcase_add_test(tc, fill);
    tcase_add_test(tc, walk_fwd);
    tcase_add_test(tc, walk_bck);
    tcase_add_test(tc, empty);

    suite_add_tcase(s, tc);

    return s;
}

#endif
