#include "bintree.h"

#include <stdint.h>
#include <assert.h>

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

int bintree_cmp(const struct bintree * const bt,
                const struct bintree_node * const a,
                const struct bintree_node * const b)
{
    return bt->cmp(bintree_element(bt, a), bintree_element(bt, b));
}

void bintree_insert(struct bintree * const bt, void * const p)
{
    struct bintree_node * const bn = (void *)((uintptr_t)p + bt->off);
    struct bintree_node ** bp = &bt->root, ** bc = bp;

    while (*bc != NULL) {
        bp = bc;

        if (bintree_cmp(bt, bn, *bp) < 0) {
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
        const int eq = bt->cmp(n, bintree_element(bt, bn));

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
    const struct bintree_node * bn,
    struct bintree_node ** (* const ch)(struct bintree_node *))
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
    struct bintree_node ** (* const ch)(struct bintree_node *),
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

int __bintree_walk(const struct bintree_node * const _bn,
                   int (* const visit)(const struct bintree_node *,
                                       bintree_visit_order_t,
                                       void *),
                   void * const priv,
                   struct bintree_node ** (* const l)(struct bintree_node *),
                   struct bintree_node ** (* const r)(struct bintree_node *))
{
    struct bintree_node * const bn = (void *)_bn;
    struct bintree_node * const ln = *l(bn), * const rn = *r(bn);
    int res = 0;

    if (res == 0 && (ln != NULL || rn != NULL)) {
        res = visit(bn, BINTREE_VISIT_ORDER_PRE, priv);
    }

    if (res == 0 && ln != NULL) {
        res = __bintree_walk(ln, visit, priv, l, r);
    }

    if (res == 0) {
        if (ln == NULL && rn == NULL) {
            res = visit(bn, BINTREE_VISIT_ORDER_LEAF, priv);
        } else {
            res = visit(bn, BINTREE_VISIT_ORDER_MID, priv);
        }
    }

    if (res == 0 && rn != NULL) {
        res = __bintree_walk(rn, visit, priv, l, r);
    }

    if (res == 0 && (ln != NULL || rn != NULL)) {
        res = visit(bn, BINTREE_VISIT_ORDER_POST, priv);
    }

    return res;
}

struct bintree_walk_priv
{
    const struct bintree * bt;
    int (* visit)(const void *, void *);
    void * priv;
};

static int bintree_walk_visit(const struct bintree_node * const bn,
                              const bintree_visit_order_t order,
                              void * const priv)
{
    int res = 0;

    if (order == BINTREE_VISIT_ORDER_MID
        || order == BINTREE_VISIT_ORDER_LEAF) {
        struct bintree_walk_priv * const bwp = priv;
        res = bwp->visit(bintree_element(bwp->bt, bn), bwp->priv);
    }

    return res;
}

int bintree_walk(const struct bintree * const bt,
                 int (* const visit)(const void *, void *),
                 void * const priv,
                 const bintree_walk_dir_t dir)
{
    int res = 0;

    if (bt->root != NULL) {
        struct bintree_walk_priv bwp;

        bwp.bt = bt;
        bwp.visit = visit;
        bwp.priv = priv;

        switch (dir) {
        case BINTREE_WALK_DIR_FWD:
            res = __bintree_walk(bt->root, bintree_walk_visit, &bwp,
                                 __bintree_left, __bintree_right);
            break;
        case BINTREE_WALK_DIR_REV:
            res = __bintree_walk(bt->root, bintree_walk_visit, &bwp,
                                 __bintree_right, __bintree_left);
            break;
        }
    }

    return res;
}

struct bintree_clear_priv
{
    struct bintree * bt;
    void (* visit)(void *);
};

static int __bintree_clear_visit(const struct bintree_node * const bn,
                                 const bintree_visit_order_t order,
                                 void * const p)
{
    if (order == BINTREE_VISIT_ORDER_POST
        || order == BINTREE_VISIT_ORDER_LEAF) {
        struct bintree_clear_priv * const bcp = p;
        bcp->visit(__bintree_element(bcp->bt, bn));
    }

    return 0;
}

void bintree_clear(struct bintree * const bt, void (* const visit)(void *))
{
    if (bt->root != NULL) {
        struct bintree_clear_priv bcp;

        bcp.bt = bt;
        bcp.visit = visit;

        __bintree_walk(bt->root, __bintree_clear_visit, &bcp,
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

        __bintree_walk(bt->root, __bintree_height, &hp,
                       __bintree_left, __bintree_right);
    }

    *min = hp.min;
    *max = hp.max;
}

void __bintree_rotate(
    struct bintree * const bt, struct bintree_node * const x,
    struct bintree_node ** (* const l)(struct bintree_node *),
    struct bintree_node ** (* const r)(struct bintree_node *))
{
    struct bintree_node * const y = *r(x);
    assert(y != NULL);

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
            ck_assert_int_lt(bintree_cmp(bt, bn->l, bn), 0);
        }
        if (bn->r != NULL) {
            ck_assert_int_ge(bintree_cmp(bt, bn->r, bn), 0);
        }
    }

    return 0;
}

static void bintree_verify(const struct bintree * const bt)
{
    if (bt->root != NULL) {
        __bintree_walk(bt->root, __bintree_verify, (void *)bt,
                       __bintree_left, __bintree_right);
    }
}

struct integer {
    int v;
    struct bintree_node bn;
};

static int cmp_integer(const void * const a, const void * const b)
{
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

START_TEST(init)
{
    struct bintree bt;

    BINTREE_INIT(&bt, struct integer, bn, cmp_integer);
}
END_TEST

static void __test__bintree_fill(struct bintree * const bt, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * const in = malloc(sizeof(*in));

        do {
            in->v = rand() % n;
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

    BINTREE_INIT(&bt, struct integer, bn, cmp_integer);
    __test__bintree_fill(&bt, n);
    {
        size_t min, max;
        bintree_height(&bt, &min, &max);
    }
    __test__bintree_drain(&bt);
}
END_TEST

static int __test__walk_fwd_visit(const void * const v, void * const p)
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

    BINTREE_INIT(&bt, struct integer, bn, cmp_integer);
    __test__bintree_fill(&bt, n);

    i = 0;
    bintree_walk(&bt, __test__walk_fwd_visit, &i, 0);

    bintree_clear(&bt, free);
}
END_TEST

static int __test__walk_rev_visit(const void * const v, void * const p)
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

    BINTREE_INIT(&bt, struct integer, bn, cmp_integer);
    __test__bintree_fill(&bt, n);

    i = n;
    bintree_walk(&bt, __test__walk_rev_visit, &i, 1);

    bintree_clear(&bt, free);
}
END_TEST

START_TEST(random_empty)
{
    static const size_t n = 100;

    struct bintree bt;

    BINTREE_INIT(&bt, struct integer, bn, cmp_integer);
    __test__bintree_fill(&bt, n);

    size_t sz;

    while ((sz = bintree_size(&bt)) > 0) {
        struct integer _in, * in;

        _in.v = rand() % n;

        in = bintree_erase(&bt, &_in);
        if (in != NULL) {
            free(in);
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
