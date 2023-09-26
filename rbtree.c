#include "rbtree.h"

#include <stdint.h>
#include <assert.h>

void rbtree_init(struct rbtree * const t,
                 compar_t * const cmp, const size_t off)
{
    bintree_init(&t->t, cmp, off + offsetof(struct rbtree_node, n));
    t->off = off;
}

static const void * rbtree_element(const struct rbtree_node * const n,
                                   const size_t off)
{
    return (void *)((uintptr_t)n - off);
}

#define BN_COLOR(BN)                                            \
    ((struct rbtree_node *)(                                    \
        (uintptr_t)BN - offsetof(struct rbtree_node, n)))->c

#define RBTREE_FIX_INSERTION(T, X, L, R)                        \
    do {                                                        \
        struct bintree_node * const y = X->p->p->R;             \
                                                                \
        if (y != NULL && BN_COLOR(y) == RBTREE_COLOR_R) {       \
            BN_COLOR(X->p) = RBTREE_COLOR_B;                    \
            BN_COLOR(y) = RBTREE_COLOR_B;                       \
            BN_COLOR(X->p->p) = RBTREE_COLOR_R;                 \
            X = X->p->p;                                        \
        } else {                                                \
            if (X == X->p->R) {                                 \
                X = X->p;                                       \
                __bintree_rotate_##L(T, X);                     \
            }                                                   \
            BN_COLOR(X->p) = RBTREE_COLOR_B;                    \
            BN_COLOR(X->p->p) = RBTREE_COLOR_R;                 \
            __bintree_rotate_##R(T, X->p->p);                   \
        }                                                       \
    } while (0)                                                 \

void rbtree_insert(struct rbtree * const t, struct rbtree_node * const n)
{
    struct bintree_node * x;

    bintree_insert(&t->t, &n->n);
    n->c = RBTREE_COLOR_R;

    x = &n->n;
    while (x->p != NULL && BN_COLOR(x->p) == RBTREE_COLOR_R) {
        if (x->p == x->p->p->l) {
            RBTREE_FIX_INSERTION(&t->t, x, l, r);
        } else {
            RBTREE_FIX_INSERTION(&t->t, x, r, l);
        }
    }

    BN_COLOR(t->t.root) = RBTREE_COLOR_B;
}

#define RBTREE_FIX_DELETION(T, X, L, R)                                 \
    do {                                                                \
        struct bintree_node * w;                                        \
                                                                        \
        w = X->p->R;                                                    \
        if (BN_COLOR(w) == RBTREE_COLOR_R) {                            \
            BN_COLOR(w) = RBTREE_COLOR_B;                               \
            BN_COLOR(X->p) = RBTREE_COLOR_R;                            \
            __bintree_rotate_##L(T, X->p);                              \
            w = X->p->R;                                                \
        }                                                               \
                                                                        \
        if ((w->L == NULL || BN_COLOR(w->L) == RBTREE_COLOR_B)          \
            && (w->R == NULL || BN_COLOR(w->R) == RBTREE_COLOR_B)) {    \
            BN_COLOR(w) = RBTREE_COLOR_R;                               \
            X = X->p;                                                   \
        } else {                                                        \
            if (w->R == NULL || BN_COLOR(w->R) == RBTREE_COLOR_B) {     \
                BN_COLOR(w->L) = RBTREE_COLOR_B;                        \
                BN_COLOR(w) = RBTREE_COLOR_R;                           \
                __bintree_rotate_##R(T, w);                             \
                w = X->p->R;                                            \
            }                                                           \
                                                                        \
            BN_COLOR(w) = BN_COLOR(X->p);                               \
            BN_COLOR(X->p) = RBTREE_COLOR_B;                            \
            BN_COLOR(w->R) = RBTREE_COLOR_B;                            \
            __bintree_rotate_##L(T, X->p);                              \
            x = (T)->root;                                              \
        }                                                               \
    } while (0)                                                         \

void rbtree_erase(struct rbtree * const t, struct rbtree_node * const n)
{
    struct bintree_node * const y = __bintree_erase(&t->t, &n->n);
    const rbtree_color_t c = BN_COLOR(y);

    BN_COLOR(y) = n->c;

    if (c == RBTREE_COLOR_B) {
        struct rbtree_node _x;
        struct bintree_node * x;

        assert(n->n.l == NULL || n->n.r == NULL);

        if (n->n.l != NULL) {
            x = n->n.l;
        } else if (n->n.r != NULL) {
            x = n->n.r;
        } else {
            x = &_x.n;

            x->p = n->n.p;
            BN_COLOR(x) = RBTREE_COLOR_B;
        }

        assert(x->p == n->n.p);

        while (x->p != NULL && BN_COLOR(x) == RBTREE_COLOR_B) {
            if (x == x->p->l || (x == &_x.n && x->p->l == NULL)) {
                RBTREE_FIX_DELETION(&t->t, x, l, r);
            } else {
                RBTREE_FIX_DELETION(&t->t, x, r, l);
            }
        }

        BN_COLOR(x) = RBTREE_COLOR_B;
    }
}

#ifdef __test__
#include <check.h>
#include <stdlib.h>

struct integer {
    struct rbtree_node n;
    int v;
};

static int cmp_integer(const void * const a, const void * const b)
{
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

START_TEST(init)
{
    struct rbtree bt;

    rbtree_init(&bt, cmp_integer, offsetof(struct integer, n));
}
END_TEST

int __rbtree_verify(const struct bintree_node * const bn, void * const p)
{
    const struct bintree * const t = p;

    size_t bh = 0;

    if (BN_COLOR(bn) == RBTREE_COLOR_R) {
        ck_assert(bn->l == NULL || BN_COLOR(bn->l) == RBTREE_COLOR_B);
        ck_assert(bn->r == NULL || BN_COLOR(bn->r) == RBTREE_COLOR_B);
    }

    if (bn->l != NULL) {
        ck_assert_int_lt(
            __bintree_cmp(bn->l, bn, t->cmp, t->off), 0);
    }
    if (bn->r != NULL) {
        ck_assert_int_ge(
            __bintree_cmp(bn->r, bn, t->cmp, t->off), 0);
    }

    if (bn->l == NULL && bn->r == NULL) {
        const struct bintree_node * n;
        size_t h;

        for (h = 0, n = bn; n != NULL; n = n->p) {
            if (BN_COLOR(n) == RBTREE_COLOR_B) {
                h++;
            }
        }

        ck_assert(bh == 0 || h == bh);
        bh = h;
    }

    return 0;
}

void rbtree_verify(const struct rbtree * const t)
{
    if (t->t.root != NULL) {
        size_t min, max;

        bintree_height(&t->t, &min, &max);
        ck_assert_uint_le(max, 2 * log2(rbtree_size(t) + 1));
        ck_assert_uint_le(max, 2 * min);

        __bintree_walk(t->t.root, __rbtree_verify, (void *)&t->t);
    }
}

static void __test__rbtree_fill(struct rbtree * const t, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * const in = malloc(sizeof(*in));
        in->v = i;

        rbtree_insert(t, &in->n);
        ck_assert_uint_eq(i + 1, rbtree_size(t));
    }
}

static void __test__rbtree_drain(struct rbtree * const t)
{
    size_t sz;

    while ((sz = rbtree_size(t)) > 0) {
        struct rbtree_node * n =
            (struct rbtree_node *)((uintptr_t)t->t.root -
                                   offsetof(struct rbtree_node, n));

        rbtree_erase(t, n);
        free((void *)rbtree_element(n, t->off));

        ck_assert_uint_eq(sz - 1, rbtree_size(t));
    }

    ck_assert_ptr_null(t->t.root);
    ck_assert_uint_eq(rbtree_size(t), 0);
}

START_TEST(fill)
{
    static const size_t n = 100;

    struct rbtree t;

    rbtree_init(&t, cmp_integer, offsetof(struct integer, n));
    __test__rbtree_fill(&t, n);
    rbtree_verify(&t);
    __test__rbtree_drain(&t);
}
END_TEST

START_TEST(random_fill)
{
    static const size_t n = 100;

    struct rbtree t;
    unsigned int i;

    rbtree_init(&t, cmp_integer, offsetof(struct integer, n));

    for (i = 0; i < n; i++) {
        struct integer * const in = malloc(sizeof(*in));

        do {
            in->v = rand() % n;
        } while (rbtree_find(&t, in) != NULL);

        rbtree_insert(&t, &in->n);
        ck_assert_uint_eq(i + 1, rbtree_size(&t));
    }

    rbtree_verify(&t);
    __test__rbtree_drain(&t);
}
END_TEST

START_TEST(random_empty)
{
    static const size_t n = 100;

    struct rbtree t;

    rbtree_init(&t, cmp_integer, offsetof(struct integer, n));
    __test__rbtree_fill(&t, n);

    size_t sz;
    while ((sz = rbtree_size(&t)) > 0) {
        struct integer in, * f;

        in.v = rand() % n;
        f = (void *)rbtree_find(&t, &in);

        if (f != NULL) {
            rbtree_erase(&t, &f->n);
            free((void *)f);
            ck_assert_uint_eq(sz - 1, rbtree_size(&t));

            rbtree_verify(&t);
        }
    }

    ck_assert_ptr_null(t.t.root);
    ck_assert_uint_eq(t.t.count, 0);
}
END_TEST

Suite * rbtree_suite(void)
{
    Suite * const s = suite_create("rbtree");

    TCase * tc;

    tc = tcase_create("rbtree");
    tcase_add_test(tc, init);
    tcase_add_test(tc, fill);
    tcase_add_test(tc, random_fill);
    tcase_add_test(tc, random_empty);

    suite_add_tcase(s, tc);

    return s;
}

#endif
