#include "bintree.h"

static void * __bintree_element(
    const struct bintree * const bt, const struct bintree_node * const bn)
{
    return (void *)((uintptr_t)bn - bt->off);
}

static const void * bintree_element(
    const struct bintree * const bt, const struct bintree_node * const bn)
{
    return (void *)((uintptr_t)bn - bt->off);
}

int __bintree_cmp(const struct bintree * const bt,
                  const struct bintree_node * const a,
                  const struct bintree_node * const b)
{
    return bt->cmp.func(bintree_element(bt, a),
                        bintree_element(bt, b),
                        bt->cmp.priv);
}

void bintree_insert(struct bintree * const bt, void * const p)
{
    struct bintree_node * const bn = (void *)((uintptr_t)p + bt->off);
    struct bintree_node ** bp = &bt->root, ** bc = bp;

    while (*bc != NULL) {
        bp = bc;

        if (__bintree_cmp(bt, bn, *bp) < 0) {
            bc = &(*bp)->l;
        } else {
            bc = &(*bp)->r;
        }
    }

    bn->p = *bp;
    bn->l = NULL;
    bn->r = NULL;

    *bc = bn;

    bt->size++;
}

const void * bintree_find(const struct bintree * const bt, const void * n)
{
    struct bintree_node * bn = bt->root;

    while (bn != NULL) {
        const int eq = bt->cmp.func(n, bintree_element(bt, bn), bt->cmp.priv);

        if (eq < 0) {
            bn = bn->l;
        } else if (eq > 0) {
            bn = bn->r;
        } else {
            break;
        }
    }

    if (bn != NULL) {
        return bintree_element(bt, bn);
    }

    return NULL;
}

static inline struct bintree_node * bintree_slide(
    const struct bintree_node * bn, bintree_child_func_t * const ch)
{
    while (*ch((struct bintree_node *)bn) != NULL) {
        bn = *ch((struct bintree_node *)bn);
    }

    return (struct bintree_node *)bn;
}

/* same as bintree_first(), but cannot return NULL */
static struct bintree_node * __bintree_lmost(const struct bintree_node * bn)
{
    return bintree_slide(bn, __bintree_left);
}

/* same as bintree_last(), but cannot return NULL */
static struct bintree_node * __bintree_rmost(const struct bintree_node * bn)
{
    return bintree_slide(bn, __bintree_right);
}

static struct bintree_node * __bintree_adjacent(
    const struct bintree_node * bn,
    bintree_child_func_t * const ch,
    struct bintree_node * (* const lrmost)(const struct bintree_node *))
{
    if (*ch((struct bintree_node *)bn) != NULL) {
        bn = lrmost(*ch((struct bintree_node *)bn));
    } else {
        while (bn->p != NULL && *ch((struct bintree_node *)bn->p) == bn) {
            bn = bn->p;
        }

        bn = bn->p;
    }

    return (struct bintree_node *)bn;
}

/* could return NULL */
static struct bintree_node * __bintree_next(const struct bintree_node * bn)
{
    return __bintree_adjacent(bn, __bintree_right, __bintree_lmost);
}

/* could return NULL */
static struct bintree_node * __bintree_prev(const struct bintree_node * bn)
{
    return __bintree_adjacent(bn, __bintree_left, __bintree_rmost);
}

const struct bintree_node * __bintree_erase(struct bintree * const bt,
                                            struct bintree_node * const bn)
{
    struct bintree_node * x, * y;

    if (bn->l != NULL && bn->r != NULL) {
        y = __bintree_next(bn);
    } else {
        y = bn;
    }

    cstl_assert(y->l == NULL || y->r == NULL);

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

    bt->size--;

    return y;
}

void * bintree_erase(struct bintree * const bt, const void * const _p)
{
    void * p = (void *)bintree_find(bt, _p);

    if (p != NULL) {
        (void)__bintree_erase(bt, (void *)((uintptr_t)p + bt->off));
    }

    return p;
}

int __bintree_foreach(const struct bintree_node * const _bn,
                      int (* const visit)(const struct bintree_node *,
                                          bintree_visit_order_t,
                                          void *),
                      void * const priv,
                      bintree_child_func_t * const l,
                      bintree_child_func_t * const r)
{
    struct bintree_node * const bn = (void *)_bn;
    struct bintree_node * const ln = *l(bn), * const rn = *r(bn);
    int res = 0;

    if (res == 0 && (ln != NULL || rn != NULL)) {
        res = visit(bn, BINTREE_VISIT_ORDER_PRE, priv);
    }

    if (res == 0 && ln != NULL) {
        res = __bintree_foreach(ln, visit, priv, l, r);
    }

    if (res == 0) {
        if (ln == NULL && rn == NULL) {
            res = visit(bn, BINTREE_VISIT_ORDER_LEAF, priv);
        } else {
            res = visit(bn, BINTREE_VISIT_ORDER_MID, priv);
        }
    }

    if (res == 0 && rn != NULL) {
        res = __bintree_foreach(rn, visit, priv, l, r);
    }

    if (res == 0 && (ln != NULL || rn != NULL)) {
        res = visit(bn, BINTREE_VISIT_ORDER_POST, priv);
    }

    return res;
}

struct bintree_foreach_priv
{
    const struct bintree * bt;
    cstl_const_visit_func_t * visit;
    void * priv;
};

static int bintree_foreach_visit(const struct bintree_node * const bn,
                                 const bintree_visit_order_t order,
                                 void * const priv)
{
    int res = 0;

    if (order == BINTREE_VISIT_ORDER_MID
        || order == BINTREE_VISIT_ORDER_LEAF) {
        struct bintree_foreach_priv * const bfp = priv;
        res = bfp->visit(bintree_element(bfp->bt, bn), bfp->priv);
    }

    return res;
}

int bintree_foreach(const struct bintree * const bt,
                    cstl_const_visit_func_t * const visit,
                    void * const priv,
                    const bintree_foreach_dir_t dir)
{
    int res = 0;

    if (bt->root != NULL) {
        struct bintree_foreach_priv bfp;

        bfp.bt = bt;
        bfp.visit = visit;
        bfp.priv = priv;

        switch (dir) {
        case BINTREE_WALK_DIR_FWD:
            res = __bintree_foreach(bt->root, bintree_foreach_visit, &bfp,
                                    __bintree_left, __bintree_right);
            break;
        case BINTREE_WALK_DIR_REV:
            res = __bintree_foreach(bt->root, bintree_foreach_visit, &bfp,
                                    __bintree_right, __bintree_left);
            break;
        }
    }

    return res;
}

void bintree_swap(struct bintree * const a, struct bintree * const b)
{
    struct bintree t;
    cstl_swap(a, b, &t, sizeof(t));
    /*
     * the tree points to the root node, but
     * the parent pointer of the root node
     * is NULL, so no need to do any more
     */
}

struct bintree_clear_priv
{
    struct bintree * bt;
    cstl_clear_func_t * clr;
};

static int __bintree_clear_visit(const struct bintree_node * const bn,
                                 const bintree_visit_order_t order,
                                 void * const p)
{
    if (order == BINTREE_VISIT_ORDER_POST
        || order == BINTREE_VISIT_ORDER_LEAF) {
        struct bintree_clear_priv * const bcp = p;
        bcp->clr(__bintree_element(bcp->bt, bn), NULL);
    }

    return 0;
}

void bintree_clear(struct bintree * const bt, cstl_clear_func_t * const clr)
{
    if (bt->root != NULL) {
        struct bintree_clear_priv bcp;

        bcp.bt = bt;
        bcp.clr = clr;

        __bintree_foreach(bt->root, __bintree_clear_visit, &bcp,
                          __bintree_left, __bintree_right);

        bt->root  = NULL;
        bt->size = 0;
    }
}

struct bintree_height_priv
{
    size_t min, max;
};

static int __bintree_height(const struct bintree_node * bn,
                            const bintree_visit_order_t order,
                            void * const priv)
{
    struct bintree_height_priv * const hp = priv;

    if (order == BINTREE_VISIT_ORDER_LEAF) {
        size_t h;

        for (h = 0; bn != NULL; h++, bn = bn->p)
            ;

        if (h < hp->min) {
            hp->min = h;
        }

        if (h > hp->max) {
            hp->max = h;
        }
    }

    return 0;
}

void bintree_height(const struct bintree * const bt,
                    size_t * const min, size_t * const max)
{
    struct bintree_height_priv hp;

    hp.min = 0;
    hp.max = 0;

    if (bt->root != NULL) {
        hp.min = SIZE_MAX;
        hp.max = 0;

        __bintree_foreach(bt->root, __bintree_height, &hp,
                          __bintree_left, __bintree_right);
    }

    *min = hp.min;
    *max = hp.max;
}

void __bintree_rotate(
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

#ifdef __cfg_test__
#include <check.h>
#include <stdlib.h>

static int __bintree_verify(const struct bintree_node * const bn,
                            const bintree_visit_order_t order,
                            void * const priv)
{
    if (order == BINTREE_VISIT_ORDER_MID
        || order == BINTREE_VISIT_ORDER_LEAF) {
        const struct bintree * const bt = priv;

        if (bn->l != NULL) {
            ck_assert_int_lt(__bintree_cmp(bt, bn->l, bn), 0);
        }
        if (bn->r != NULL) {
            ck_assert_int_ge(__bintree_cmp(bt, bn->r, bn), 0);
        }
    }

    return 0;
}

static void bintree_verify(const struct bintree * const bt)
{
    if (bt->root != NULL) {
        __bintree_foreach(bt->root, __bintree_verify, (void *)bt,
                          __bintree_left, __bintree_right);
    }
}

struct integer {
    int v;
    struct bintree_node bn;
};

static int cmp_integer(const void * const a, const void * const b,
                       void * const p)
{
    (void)p;
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

START_TEST(init)
{
    struct bintree bt;

    BINTREE_INIT(&bt, struct integer, bn, cmp_integer, NULL);
}
END_TEST

static void __test_bintree_free(void * const p, void * const x)
{
    (void)x;
    cstl_free(p);
}

static void __test__bintree_fill(struct bintree * const bt, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * const in = cstl_malloc(sizeof(*in));

        do {
            in->v = cstl_rand() % n;
        } while (bintree_find(bt, in) != NULL);

        bintree_insert(bt, in);
        ck_assert_uint_eq(i + 1, bintree_size(bt));

        bintree_verify(bt);
    }
}

static void __test__bintree_drain(struct bintree * const bt)
{
    size_t sz;

    while ((sz = bintree_size(bt)) > 0) {
        struct bintree_node * bn = bt->root;

        __bintree_erase(bt, bn);
        cstl_free((void *)bintree_element(bt, bn));

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

    BINTREE_INIT(&bt, struct integer, bn, cmp_integer, NULL);
    __test__bintree_fill(&bt, n);
    {
        size_t min, max;
        bintree_height(&bt, &min, &max);
    }
    __test__bintree_drain(&bt);
}
END_TEST

static int __test__foreach_fwd_visit(const void * const v, void * const p)
{
    const struct integer * const in = v;
    unsigned int * const i = p;

    ck_assert_uint_eq(*i, in->v);
    (*i)++;

    return 0;
}

START_TEST(walk_fwd)
{
    static const size_t n = 100;

    struct bintree bt;
    unsigned int i;

    BINTREE_INIT(&bt, struct integer, bn, cmp_integer, NULL);
    __test__bintree_fill(&bt, n);

    i = 0;
    bintree_foreach(&bt, __test__foreach_fwd_visit, &i, 0);

    bintree_clear(&bt, __test_bintree_free);
}
END_TEST

static int __test__foreach_rev_visit(const void * const v, void * const p)
{
    const struct integer * const in = v;
    unsigned int * const i = p;

    (*i)--;
    ck_assert_uint_eq(*i, in->v);

    return 0;
}

START_TEST(walk_rev)
{
    static const size_t n = 100;

    struct bintree bt;
    unsigned int i;

    BINTREE_INIT(&bt, struct integer, bn, cmp_integer, NULL);
    __test__bintree_fill(&bt, n);

    i = n;
    bintree_foreach(&bt, __test__foreach_rev_visit, &i, 1);

    bintree_clear(&bt, __test_bintree_free);
}
END_TEST

START_TEST(random_empty)
{
    static const size_t n = 100;

    struct bintree bt;

    BINTREE_INIT(&bt, struct integer, bn, cmp_integer, NULL);
    __test__bintree_fill(&bt, n);

    size_t sz;

    while ((sz = bintree_size(&bt)) > 0) {
        struct integer _in, * in;

        _in.v = cstl_rand() % n;

        in = bintree_erase(&bt, &_in);
        if (in != NULL) {
            cstl_free(in);
            ck_assert_uint_eq(sz - 1, bintree_size(&bt));
        }
    }

    ck_assert_ptr_null(bt.root);
    ck_assert_uint_eq(bt.size, 0);
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
    tcase_add_test(tc, walk_rev);
    tcase_add_test(tc, random_empty);

    suite_add_tcase(s, tc);

    return s;
}

#endif
