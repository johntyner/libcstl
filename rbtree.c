#include "rbtree.h"

static const void * rbtree_element(const struct rbtree * const t,
                                   const struct rbtree_node * const n)
{
    return (void *)((uintptr_t)n - t->off);
}

static inline rbtree_color_t * BN_COLOR(const struct bintree_node * const bn)
{
    return &((struct rbtree_node *)(
                 (uintptr_t)bn - offsetof(struct rbtree_node, n)))->c;
}

static void __bintree_rotate(
    struct bintree * const bt, struct bintree_node * const x,
    bintree_child_func_t * const l, bintree_child_func_t * const r)
{
    struct bintree_node * const y = *r(x);
    cstl_assert(y != NULL);

    *r(x) = *l(y);
    if (*l(y) != NULL) {
        (*l(y))->p = x;
    }
    y->p = x->p;
    if (x->p == NULL) {
        bt->root = y;
    } else if (x == *l(x->p)) {
        *l(x->p) = y;
    } else {
        *r(x->p) = y;
    }
    *l(y) = x;
    x->p = y;
}

static struct bintree_node * rbtree_fix_insertion(
    struct bintree * const t, struct bintree_node * x,
    bintree_child_func_t * const l, bintree_child_func_t * const r)
{
    struct bintree_node * const y = *r(x->p->p);

    if (y != NULL && *BN_COLOR(y) == RBTREE_COLOR_R) {
        *BN_COLOR(x->p) = RBTREE_COLOR_B;
        *BN_COLOR(y) = RBTREE_COLOR_B;
        *BN_COLOR(x->p->p) = RBTREE_COLOR_R;
        x = x->p->p;
    } else {
        if (x == *r(x->p)) {
            x = x->p;
            __bintree_rotate(t, x, l, r);
        }
        *BN_COLOR(x->p) = RBTREE_COLOR_B;
        *BN_COLOR(x->p->p) = RBTREE_COLOR_R;
        __bintree_rotate(t, x->p->p, r, l);
    }

    return x;
}

void rbtree_insert(struct rbtree * const t, void * const p)
{
    struct rbtree_node * const n = (void *)((uintptr_t)p + t->off);
    struct bintree_node * x;

    bintree_insert(&t->t, p);
    n->c = RBTREE_COLOR_R;

    x = &n->n;
    while (x->p != NULL && *BN_COLOR(x->p) == RBTREE_COLOR_R) {
        if (x->p == x->p->p->l) {
            x = rbtree_fix_insertion(
                &t->t, x,
                __bintree_left, __bintree_right);
        } else {
            x = rbtree_fix_insertion(
                &t->t, x,
                __bintree_right, __bintree_left);
        }
    }

    *BN_COLOR(t->t.root) = RBTREE_COLOR_B;
}

static struct bintree_node * rbtree_fix_deletion(
    struct bintree * const t, struct bintree_node * x,
    bintree_child_func_t * const l, bintree_child_func_t * const r)
{
    struct bintree_node * w;

    w = *r(x->p);
    if (*BN_COLOR(w) == RBTREE_COLOR_R) {
        *BN_COLOR(w) = RBTREE_COLOR_B;
        *BN_COLOR(x->p) = RBTREE_COLOR_R;
        __bintree_rotate(t, x->p, l, r);
        w = *r(x->p);
    }

    if ((*l(w) == NULL || *BN_COLOR(*l(w)) == RBTREE_COLOR_B)
        && (*r(w) == NULL || *BN_COLOR(*r(w)) == RBTREE_COLOR_B)) {
        *BN_COLOR(w) = RBTREE_COLOR_R;
        x = x->p;
    } else {
        if (*r(w) == NULL || *BN_COLOR(*r(w)) == RBTREE_COLOR_B) {
            *BN_COLOR(*l(w)) = RBTREE_COLOR_B;
            *BN_COLOR(w) = RBTREE_COLOR_R;
            __bintree_rotate(t, w, r, l);
            w = *r(x->p);
        }

        *BN_COLOR(w) = *BN_COLOR(x->p);
        *BN_COLOR(x->p) = RBTREE_COLOR_B;
        *BN_COLOR(*r(w)) = RBTREE_COLOR_B;
        __bintree_rotate(t, x->p, l, r);
        x = t->root;
    }

    return x;
}

void * rbtree_erase(struct rbtree * const t, const void * const _p)
{
    void * const p = (void *)rbtree_find(t, _p);

    if (p != NULL) {
        struct rbtree_node * const n = (void *)((uintptr_t)p + t->off);
        const struct bintree_node * const y = __bintree_erase(&t->t, &n->n);
        const rbtree_color_t c = *BN_COLOR(y);

        *BN_COLOR(y) = n->c;

        if (c == RBTREE_COLOR_B) {
            struct rbtree_node _x;
            struct bintree_node * x;

            cstl_assert(n->n.l == NULL || n->n.r == NULL);

            if (n->n.l != NULL) {
                x = n->n.l;
            } else if (n->n.r != NULL) {
                x = n->n.r;
            } else {
                x = &_x.n;

                x->p = n->n.p;
                *BN_COLOR(x) = RBTREE_COLOR_B;
            }

            cstl_assert(x->p == n->n.p);

            while (x->p != NULL && *BN_COLOR(x) == RBTREE_COLOR_B) {
                if (x == x->p->l || (x == &_x.n && x->p->l == NULL)) {
                    x = rbtree_fix_deletion(
                        &t->t, x,
                        __bintree_left, __bintree_right);
                } else {
                    x = rbtree_fix_deletion(
                        &t->t, x,
                        __bintree_right, __bintree_left);
                }
            }

            *BN_COLOR(x) = RBTREE_COLOR_B;
        }
    }

    return p;
}

#ifdef __cfg_test__
#include <check.h>
#include <stdlib.h>

struct integer {
    int v;
    struct rbtree_node n;
};

static int cmp_integer(const void * const a, const void * const b,
                       void * const p)
{
    (void)p;
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

START_TEST(init)
{
    struct rbtree t;

    RBTREE_INIT(&t, struct integer, n, cmp_integer, NULL);
}
END_TEST

static int __rbtree_verify(const void * const elem, void * const priv)
{
    const struct bintree * const t = priv;
    const struct bintree_node * const bn =
        &((const struct integer *)elem)->n.n;

    size_t bh = 0;

    if (*BN_COLOR(bn) == RBTREE_COLOR_R) {
        ck_assert(bn->l == NULL || *BN_COLOR(bn->l) == RBTREE_COLOR_B);
        ck_assert(bn->r == NULL || *BN_COLOR(bn->r) == RBTREE_COLOR_B);
    }

    if (bn->l != NULL) {
        ck_assert_int_lt(__bintree_cmp(t, bn->l, bn), 0);
    }
    if (bn->r != NULL) {
        ck_assert_int_ge(__bintree_cmp(t, bn->r, bn), 0);
    }

    if (bn->l == NULL && bn->r == NULL) {
        const struct bintree_node * n;
        size_t h;

        for (h = 0, n = bn; n != NULL; n = n->p) {
            if (*BN_COLOR(n) == RBTREE_COLOR_B) {
                h++;
            }
        }

        ck_assert(bh == 0 || h == bh);
        bh = h;
    }

    return 0;
}

static void rbtree_verify(const struct rbtree * const t)
{
    if (t->t.root != NULL) {
        size_t min, max;

        bintree_height(&t->t, &min, &max);
        ck_assert_uint_le(max, 2 * log2(rbtree_size(t) + 1));
        ck_assert_uint_le(max, 2 * min);

        bintree_foreach(&t->t,
                        __rbtree_verify, (void *)&t->t,
                        BINTREE_WALK_DIR_FWD);
    }
}

static void __test_rbtree_free(void * const p, void * const x)
{
    (void)x;
    cstl_free(p);
}

static void __test__rbtree_fill(struct rbtree * const t, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * const in = cstl_malloc(sizeof(*in));
        in->v = i;

        rbtree_insert(t, in);
        ck_assert_uint_eq(i + 1, rbtree_size(t));
    }
}

START_TEST(fill)
{
    static const size_t n = 100;

    struct rbtree t;

    RBTREE_INIT(&t, struct integer, n, cmp_integer, NULL);
    __test__rbtree_fill(&t, n);
    rbtree_verify(&t);
    rbtree_clear(&t, __test_rbtree_free);
}
END_TEST

START_TEST(random_fill)
{
    static const size_t n = 100;

    struct rbtree t;
    unsigned int i;

    RBTREE_INIT(&t, struct integer, n, cmp_integer, NULL);

    for (i = 0; i < n; i++) {
        struct integer * const in = cstl_malloc(sizeof(*in));

        do {
            in->v = cstl_rand() % n;
        } while (rbtree_find(&t, in) != NULL);

        rbtree_insert(&t, in);
        ck_assert_uint_eq(i + 1, rbtree_size(&t));
    }

    rbtree_verify(&t);
    rbtree_clear(&t, __test_rbtree_free);
}
END_TEST

START_TEST(random_empty)
{
    static const size_t n = 100;

    struct rbtree t;

    RBTREE_INIT(&t, struct integer, n, cmp_integer, NULL);
    __test__rbtree_fill(&t, n);

    size_t sz;
    while ((sz = rbtree_size(&t)) > 0) {
        struct integer _in, * in;

        _in.v = cstl_rand() % n;

        in = rbtree_erase(&t, &_in);
        if (in != NULL) {
            cstl_free(in);
            ck_assert_uint_eq(sz - 1, rbtree_size(&t));

            rbtree_verify(&t);
        }
    }

    ck_assert_ptr_null(t.t.root);
    ck_assert_uint_eq(rbtree_size(&t), 0);
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
