#include "bintree.h"

#include <stdint.h>
#include <assert.h>

static void __bintree_insert(struct bintree_node * bp,
                             struct bintree_node * const bn,
                             compar_t * const cmp, const size_t off)
{
    struct bintree_node ** bc = &bp;

    while (*bc != NULL) {
        bp = *bc;

        if (__bintree_cmp(bn, bp, cmp, off) < 0) {
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

static struct bintree_node * __bintree_find(
    const struct bintree_node * rt,
    const void * const n,
    compar_t * const cmp, const size_t off)
{
    while (rt != NULL) {
        const int eq = cmp(n, __bintree_element(rt, off));

        if (eq < 0) {
            rt = rt->l;
        } else if (eq > 0) {
            rt = rt->r;
        } else {
            break;
        }
    }

    return (struct bintree_node *)rt;
}

const void * bintree_find(const struct bintree * const bt, const void * n)
{
    struct bintree_node * const bn =
        __bintree_find(bt->root, n, bt->cmp, bt->off);
    if (bn != NULL) {
        return bintree_element(bt, bn);
    }

    return NULL;
}

#define BINTREE_SLIDE(BN, DIR)                  \
    do {                                        \
        while (BN->DIR != NULL) {               \
            BN = BN->DIR;                       \
        }                                       \
    } while (0)

/* same as bintree_first(), but cannot return NULL */
static struct bintree_node * __bintree_first(const struct bintree_node * bn)
{
    BINTREE_SLIDE(bn, l);
    return (struct bintree_node *)bn;
}

/* same as bintree_last(), but cannot return NULL */
static struct bintree_node * __bintree_last(const struct bintree_node * bn)
{
    BINTREE_SLIDE(bn, r);
    return (struct bintree_node *)bn;
}

const void * bintree_first(const struct bintree * const bt)
{
    if (bt->root == NULL) {
        return NULL;
    }

    return bintree_element(bt, __bintree_first(bt->root));
}

const void * bintree_last(const struct bintree * const bt)
{
    if (bt->root == NULL) {
        return NULL;
    }

    return bintree_element(bt, __bintree_last(bt->root));
}

#define BINTREE_NEXT(BN, DIR, FL)                       \
    do {                                                \
        if (BN->DIR != NULL) {                          \
            BN = FL(BN->DIR);                           \
        } else {                                        \
            while (BN->p != NULL && BN->p->DIR == BN) { \
                BN = BN->p;                             \
            }                                           \
            BN = BN->p;                                 \
        }                                               \
    } while (0)

/* could return NULL */
static struct bintree_node * __bintree_next(const struct bintree_node * bn)
{
    BINTREE_NEXT(bn, r, __bintree_first);
    return (struct bintree_node *)bn;
}

/* could return NULL */
static struct bintree_node * __bintree_prev(const struct bintree_node * bn)
{
    BINTREE_NEXT(bn, l, __bintree_last);
    return (struct bintree_node *)bn;
}

const void * bintree_next(const struct bintree * const bt,
                          const struct bintree_node * bn)
{
    if ((bn = __bintree_next(bn)) != NULL) {
        return bintree_element(bt, bn);
    }

    return NULL;
}

const void * bintree_prev(const struct bintree * const bt,
                          const struct bintree_node * bn)
{
    if ((bn = __bintree_prev(bn)) != NULL) {
        return bintree_element(bt, bn);
    }

    return NULL;
}

struct bintree_node * __bintree_erase(struct bintree * const bt,
                                      struct bintree_node * const bn)
{
    struct bintree_node * x, * y;

    if (bn->l != NULL && bn->r != NULL) {
        y = __bintree_next(bn);
    } else {
        y = bn;
    }

    assert(y->l == NULL || y->r == NULL);

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
        const struct bintree_node t = *y;

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
        *bn = t;

        /*
         * it's possible that the removed node, y, was
         * a direct descendant of bn. in this case, change
         * bn's (formerly y's) direct ancestor to be y
         * to more accurately reflect the state of things
         * to the caller
         */
        if (bn->p == bn) {
            bn->p = y;
        }
    }

    bt->count--;

    return y;
}

int __bintree_walk(const struct bintree_node * const bn,
                   int (* const visit)(const struct bintree_node *, void *),
                   void * const priv)
{
    int res = 0;

    if (res == 0 && bn->l != NULL) {
        res = __bintree_walk(bn->l, visit, priv);
    }

    if (res == 0) {
        res = visit(bn, priv);
    }

    if (res == 0 && bn->r != NULL) {
        res = __bintree_walk(bn->r, visit, priv);
    }

    return res;
}

struct bintree_walk_priv
{
    const struct bintree * bt;
    int (* visit)(const void *, void *);
    void * priv;
};

static int bintree_visit(const struct bintree_node * const bn,
                         void * const priv)
{
    struct bintree_walk_priv * const wp = priv;
    return wp->visit(bintree_element(wp->bt, bn), wp->priv);
}

int bintree_walk(const struct bintree * const bt,
                 int (* const visit)(const void *, void *),
                 void * const priv)
{
    int res = 0;

    if (bt->root != NULL) {
        struct bintree_walk_priv wp;

        wp.bt = bt;
        wp.visit = visit;
        wp.priv = priv;

        res = __bintree_walk(bt->root, bintree_visit, &wp);
    }

    return res;
}

void bintree_height(const struct bintree * const bt,
                    size_t * const min, size_t * const max)
{
    if (bt->root == NULL) {
        *min = 0;
        *max = 0;
    } else {
        const struct bintree_node * bn;

        *min = SIZE_MAX;
        *max = 0;

        for (bn = __bintree_first(bt->root);
             bn != NULL;
             bn = __bintree_next(bn)) {
            if (bn->l == NULL && bn->r == NULL) {
                const struct bintree_node * n;
                size_t h;

                for (h = 0, n = bn; n != NULL; h++, n = n->p)
                    ;

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

#define BINTREE_ROTATE(T, X, L, R)              \
    do {                                        \
        struct bintree_node * y;                \
        assert(X->R != NULL);                   \
        y = X->R;                               \
        X->R = y->L;                            \
        if (y->L != NULL) {                     \
            y->L->p = X;                        \
        }                                       \
        y->p = X->p;                            \
        if (X->p == NULL) {                     \
            T->root = y;                        \
        } else if (X == X->p->L) {              \
            X->p->L = y;                        \
        } else {                                \
            X->p->R = y;                        \
        }                                       \
        y->L = X;                               \
        X->p = y;                               \
    } while (0)

void __bintree_rotate_l(struct bintree * const bt,
                        struct bintree_node * const bn)
{
    BINTREE_ROTATE(bt, bn, l, r);
}

void __bintree_rotate_r(struct bintree * const bt,
                        struct bintree_node * const bn)
{
    BINTREE_ROTATE(bt, bn, r, l);
}

#ifdef __test__
#include <check.h>
#include <stdlib.h>

static void bintree_verify(const struct bintree * const bt)
{
    if (bt->root != NULL) {
        struct bintree_node * bn;

        for (bn = __bintree_first(bt->root);
             bn != NULL;
             bn = __bintree_next(bn)) {
            if (bn->l != NULL) {
                ck_assert_int_lt(__bintree_cmp(bn->l, bn, bt->cmp, bt->off), 0);
            }
            if (bn->r != NULL) {
                ck_assert_int_ge(__bintree_cmp(bn->r, bn, bt->cmp, bt->off), 0);
            }
        }
    }
}

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
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * const in = malloc(sizeof(*in));

        do {
            in->v = rand() % n;
        } while (__bintree_find(bt->root, in, bt->cmp, bt->off) != NULL);

        bintree_insert(bt, &in->bn);
        ck_assert_uint_eq(i + 1, bintree_size(bt));

        bintree_verify(bt);
    }
}

static void __test__bintree_drain(struct bintree * const bt)
{
    size_t sz;

    while ((sz = bintree_size(bt)) > 0) {
        struct bintree_node * bn = bt->root;

        bintree_erase(bt, bn);
        free((void *)bintree_element(bt, bn));

        ck_assert_uint_eq(sz - 1, bintree_size(bt));

        bintree_verify(bt);
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
    {
        size_t min, max;
        bintree_height(&bt, &min, &max);
    }
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

    for (i = 0, bn = __bintree_first(bt.root);
         i < n;
         i++, bn = __bintree_next(bn)) {
        const struct integer * const in = bintree_element(&bt, bn);
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

    for (i = 0, bn = __bintree_last(bt.root);
         i < n;
         i++, bn = __bintree_prev(bn)) {
        const struct integer * const in = bintree_element(&bt, bn);
        ck_assert_uint_eq(n - i - 1, in->v);
    }

    __test__bintree_drain(&bt);
}
END_TEST

START_TEST(random_empty)
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
        bn = __bintree_find(bt.root, &in, bt.cmp, bt.off);

        if (bn != NULL) {
            bintree_erase(&bt, bn);
            free((void *)bintree_element(&bt, bn));
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
    tcase_add_test(tc, random_empty);

    suite_add_tcase(s, tc);

    return s;
}

#endif
