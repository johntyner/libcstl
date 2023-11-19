/*!
 * @file
 */

#include "cstl/dlist.h"

/*! @private */
static void * __cstl_dlist_element(const struct cstl_dlist * const l,
                                   const struct cstl_dlist_node * const n)
{
    return (void *)((uintptr_t)n - l->off);
}

/*! @private */
static struct cstl_dlist_node * __cstl_dlist_node(
    const struct cstl_dlist * const l,
    const void * const e)
{
    return (void *)((uintptr_t)e + l->off);
}

/*! @private */
static struct cstl_dlist_node ** __cstl_dlist_next(
    struct cstl_dlist_node * const n)
{
    return &n->n;
}

/*! @private */
static struct cstl_dlist_node ** __cstl_dlist_prev(
    struct cstl_dlist_node * const n)
{
    return &n->p;
}

/*! @private */
static void __cstl_dlist_insert(struct cstl_dlist * const l,
                                struct cstl_dlist_node * const p,
                                struct cstl_dlist_node * const n)
{
    n->n = p->n;
    n->p = p;

    /* node after p/n points back to n */
    n->n->p = n;
    /* p points to n as its next */
    p->n = n;

    l->size++;
}

void cstl_dlist_insert(struct cstl_dlist * const l,
                       void * const pe, void * const e)
{
    __cstl_dlist_insert(l, __cstl_dlist_node(l, pe), __cstl_dlist_node(l, e));
}

/*! @private */
static void * __cstl_dlist_erase(struct cstl_dlist * const l,
                                 struct cstl_dlist_node * const n)
{
    n->n->p = n->p;
    n->p->n = n->n;

    l->size--;

    return __cstl_dlist_element(l, n);
}

void cstl_dlist_erase(struct cstl_dlist * const l, void * const e)
{
    __cstl_dlist_erase(l, __cstl_dlist_node(l, e));
}

void * cstl_dlist_front(struct cstl_dlist * const l)
{
    if (l->size > 0) {
        return __cstl_dlist_element(l, l->h.n);
    }
    return NULL;
}

void * cstl_dlist_back(struct cstl_dlist * const l)
{
    if (l->size > 0) {
        return __cstl_dlist_element(l, l->h.p);
    }
    return NULL;
}

void cstl_dlist_push_front(struct cstl_dlist * const l, void * const e)
{
    __cstl_dlist_insert(l, &l->h, __cstl_dlist_node(l, e));
}

void cstl_dlist_push_back(struct cstl_dlist * const l, void * const e)
{
    __cstl_dlist_insert(l, l->h.p, __cstl_dlist_node(l, e));
}

void * cstl_dlist_pop_front(struct cstl_dlist * const l)
{
    if (l->size > 0) {
        return __cstl_dlist_erase(l, l->h.n);
    }
    return NULL;
}

void * cstl_dlist_pop_back(struct cstl_dlist * const l)
{
    if (l->size > 0) {
        return __cstl_dlist_erase(l, l->h.p);
    }
    return NULL;
}

int cstl_dlist_foreach(struct cstl_dlist * const l,
                       cstl_visit_func_t * const visit, void * const p,
                       const cstl_dlist_foreach_dir_t dir)
{
    struct cstl_dlist_node ** (* next)(struct cstl_dlist_node *);
    struct cstl_dlist_node * c, * n;
    int res = 0;

    switch (dir) {
    default:
    case CSTL_DLIST_FOREACH_DIR_FWD:
        next = __cstl_dlist_next;
        break;
    case CSTL_DLIST_FOREACH_DIR_REV:
        next = __cstl_dlist_prev;
        break;
    }

    for (c = *next(&l->h), n = *next(c);
         res == 0 && c != &l->h;
         c = n, n = *next(c)) {
        res = visit(__cstl_dlist_element(l, c), p);
    }

    return res;
}

/*! @private */
struct cstl_dlist_find_priv
{
    cstl_compare_func_t * cmp;
    void * p;
    const void * e;
};

/*! @private */
static int cstl_dlist_find_visit(void * const e, void * const p)
{
    struct cstl_dlist_find_priv * lfp = p;

    if (lfp->cmp(lfp->e, e, lfp->p) == 0) {
        lfp->e = e;
        return 1;
    }

    return 0;
}

void * cstl_dlist_find(const struct cstl_dlist * const l,
                       const void * const e,
                       cstl_compare_func_t * const cmp, void * const cmp_p,
                       const cstl_dlist_foreach_dir_t dir)
{
    struct cstl_dlist_find_priv lfp;

    lfp.cmp = cmp;
    lfp.p   = cmp_p;
    lfp.e   = e;

    if (cstl_dlist_foreach((struct cstl_dlist *)l,
                           cstl_dlist_find_visit, &lfp, dir) > 0) {
        return (void *)lfp.e;
    }

    return NULL;
}

void cstl_dlist_swap(struct cstl_dlist * const a, struct cstl_dlist * const b)
{
    struct cstl_dlist t;

    cstl_swap(a, b, &t, sizeof(t));

#ifndef NO_DOC
#define CSTL_DLIST_SWAP_FIX(L)                  \
    do {                                        \
        if (L->size == 0) {                     \
            L->h.n = L->h.p = &L->h;            \
        } else {                                \
            L->h.n->p = L->h.p->n = &L->h;      \
        }                                       \
    } while (0)

    CSTL_DLIST_SWAP_FIX(a);
    CSTL_DLIST_SWAP_FIX(b);

#undef CSTL_DLIST_SWAP_FIX
#endif
}

void cstl_dlist_clear(struct cstl_dlist * const l,
                      cstl_xtor_func_t * const clr)
{
    while (l->size > 0) {
        clr(__cstl_dlist_erase(l, l->h.n), NULL);
    }
}

/*
 * time complexity is linear. the number of
 * loops/operations required is n/2, though.
 */
void cstl_dlist_reverse(struct cstl_dlist * const l)
{
    struct cstl_dlist_node * i, * j, * k;

    /*
     * i points to the first item in the list; j points
     * to the last. the objects at i and j are swapped,
     * and then i and j are advanced toward the center.
     * the loop terminates when i and j point to the same
     * or adjacent (with i still before j) nodes.
     *
     * k is used as a temporary variable
     */
    for (i = l->h.n, j = l->h.p;
         i != j && i->n != j;
         k = i->p, i = j->n, j = k) {
        struct cstl_dlist_node t;

        /* point the nodes surrounding i at j... */
        i->p->n = j;
        i->n->p = j;

        /* ...and the nodes surrounding j at i */
        j->n->p = i;
        j->p->n = i;

        /* then swap i and j */
        cstl_swap(i, j, &t, sizeof(t));
    }

    /*
     * if i and j point to the same node, no need to
     * swap them. if they're different, they need to
     * be swapped, but special handling is required
     */

    if (i->n == j) {
        /*
         * point the nodes before i and after j
         * at j and i, respectively
         */
        i->p->n = j;
        j->n->p = i;

        /* then fix up i and j's pointers */
        i->n = j->n;
        j->n = i;

        j->p = i->p;
        i->p = j;
    }
}

void cstl_dlist_concat(struct cstl_dlist * const d, struct cstl_dlist * const s)
{
    if (d != s && d->off == s->off && s->size > 0) {
        /* beginning of s points back at end of d */
        s->h.n->p = d->h.p;
        /* end of s points at head of d */
        s->h.p->n = &d->h;

        /* end of d points at beginning of s */
        d->h.p->n = s->h.n;
        /* head of d points back at end of s */
        d->h.p = s->h.p;

        d->size += s->size;

        /* leave the original list in a usable state */
        cstl_dlist_init(s, s->off);
    }
}

void cstl_dlist_sort(struct cstl_dlist * const l,
                     cstl_compare_func_t * const cmp, void * const priv)
{
    /* no need to sort a list with a size of one */

    if (l->size > 1) {
        struct cstl_dlist _l[2];
        struct cstl_dlist_node * t;

        /*
         * break the list l into two halves,
         * into _l[0] and _l[1]
         */

        cstl_dlist_init(&_l[0], l->off);
        cstl_dlist_init(&_l[1], l->off);

        /* find the middle of the list */
        for (t = &l->h;
             _l[0].size < l->size / 2;
             t = t->n, _l[0].size++)
            ;

        _l[0].h.n = l->h.n;
        _l[0].h.p = t;

        _l[1].h.n = t->n;
        _l[1].h.p = l->h.p;

        _l[0].h.n->p = _l[0].h.p->n = &_l[0].h;
        _l[1].h.n->p = _l[1].h.p->n = &_l[1].h;

        _l[1].size = l->size - _l[0].size;
        cstl_dlist_init(l, l->off);

        /* sort the two halves */
        cstl_dlist_sort(&_l[0], cmp, priv);
        cstl_dlist_sort(&_l[1], cmp, priv);

        /*
         * merge the two sorted halves back together by
         * comparing the nodes at the front of each list
         * and moving the smaller one to the output
         */
        while (_l[0].size > 0 && _l[1].size > 0) {
            struct cstl_dlist_node * n;
            struct cstl_dlist * ol;

            if (cmp(__cstl_dlist_element(&_l[0], _l[0].h.n),
                    __cstl_dlist_element(&_l[1], _l[1].h.n),
                    priv) <= 0) {
                ol = &_l[0];
            } else {
                ol = &_l[1];
            }

            n = ol->h.n;
            __cstl_dlist_erase(ol, n);
            __cstl_dlist_insert(l, l->h.p, n);
        }

        /* append whatever is left to the output */
        if (_l[0].size > 0) {
            cstl_dlist_concat(l, &_l[0]);
        } else {
            cstl_dlist_concat(l, &_l[1]);
        }
    }
}

#ifdef __cfg_test__
// GCOV_EXCL_START
#include <check.h>
#include <stdlib.h>

struct integer
{
    int v;
    struct cstl_dlist_node ln;
};

static int cmp_integer(const void * const a, const void * const b,
                       void * const p)
{
    (void)p;
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

static void __test_cstl_dlist_free(void * const p, void * const x)
{
    (void)x;
    free(p);
}

static void __test__cstl_dlist_fill(struct cstl_dlist * l, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * in = malloc(sizeof(*in));

        in->v = rand() % n;
        cstl_dlist_push_back(l, in);
    }

    ck_assert_uint_eq(n, cstl_dlist_size(l));
}

START_TEST(simple)
{
    DECLARE_CSTL_DLIST(l, struct integer, ln);
    struct integer a, b, c;

    a.v = 0; b.v = 1; c.v = 2;

    ck_assert_int_eq(cstl_dlist_size(&l), 0);
    ck_assert_ptr_null(cstl_dlist_front(&l));
    ck_assert_ptr_null(cstl_dlist_back(&l));

    cstl_dlist_push_front(&l, &a);
    ck_assert_int_eq(cstl_dlist_size(&l), 1);
    ck_assert_ptr_eq(cstl_dlist_front(&l), &a);
    ck_assert_ptr_eq(cstl_dlist_back(&l), &a);

    cstl_dlist_insert(&l, &a, &b);
    ck_assert_int_eq(cstl_dlist_size(&l), 2);
    ck_assert_ptr_eq(cstl_dlist_front(&l), &a);
    ck_assert_ptr_eq(cstl_dlist_back(&l), &b);

    cstl_dlist_push_back(&l, &c);
    ck_assert_int_eq(cstl_dlist_size(&l), 3);
    ck_assert_ptr_eq(cstl_dlist_front(&l), &a);
    ck_assert_ptr_eq(cstl_dlist_back(&l), &c);

    ck_assert_ptr_eq(
        cstl_dlist_find(&l,
                        &b, cmp_integer, NULL,
                        CSTL_DLIST_FOREACH_DIR_FWD),
        &b);

    ck_assert_ptr_eq(&a, cstl_dlist_pop_front(&l));
    ck_assert_int_eq(cstl_dlist_size(&l), 2);

    ck_assert_ptr_null(
        cstl_dlist_find(&l,
                        &a, cmp_integer, NULL,
                        CSTL_DLIST_FOREACH_DIR_REV));

    ck_assert_ptr_eq(&c, cstl_dlist_pop_back(&l));
    ck_assert_int_eq(cstl_dlist_size(&l), 1);
    cstl_dlist_erase(&l, &b);
    ck_assert_int_eq(cstl_dlist_size(&l), 0);

    ck_assert_ptr_null(cstl_dlist_pop_front(&l));
    ck_assert_ptr_null(cstl_dlist_pop_back(&l));
}
END_TEST

START_TEST(fill)
{
    static const size_t n = 100;
    DECLARE_CSTL_DLIST(l, struct integer, ln);

    __test__cstl_dlist_fill(&l, n);

    cstl_dlist_clear(&l, __test_cstl_dlist_free);
    ck_assert_uint_eq(cstl_dlist_size(&l), 0);
}
END_TEST

START_TEST(concat)
{
    static const size_t n = 4;
    DECLARE_CSTL_DLIST(l1, struct integer, ln);
    DECLARE_CSTL_DLIST(l2, struct integer, ln);

    __test__cstl_dlist_fill(&l1, n);
    __test__cstl_dlist_fill(&l2, n);

    cstl_dlist_concat(&l1, &l2);
    ck_assert_uint_eq(cstl_dlist_size(&l1), 2 * n);
    ck_assert_uint_eq(cstl_dlist_size(&l2), 0);

    cstl_dlist_clear(&l1, __test_cstl_dlist_free);
    cstl_dlist_clear(&l2, __test_cstl_dlist_free);
}
END_TEST

static int cstl_dlist_verify_sorted_fwd(void * const e, void * const p)
{
    struct integer ** in = p;
    if (*in != NULL) {
        ck_assert_int_ge(((struct integer *)e)->v, (*in)->v);
    }
    *in = e;
    return 0;
}

static int cstl_dlist_verify_sorted_rev(void * const e, void * const p)
{
    struct integer ** in = p;
    if (*in != NULL) {
        ck_assert_int_le(((struct integer *)e)->v, (*in)->v);
    }
    *in = e;
    return 0;
}

START_TEST(sort)
{
    static const size_t n = 100;
    DECLARE_CSTL_DLIST(l, struct integer, ln);

    struct integer * in;

    __test__cstl_dlist_fill(&l, n);

    cstl_dlist_sort(&l, cmp_integer, NULL);
    ck_assert_uint_eq(n, cstl_dlist_size(&l));
    in = NULL;
    cstl_dlist_foreach(&l,
                       cstl_dlist_verify_sorted_fwd, &in,
                       CSTL_DLIST_FOREACH_DIR_FWD);
    in = NULL;
    cstl_dlist_foreach(&l,
                       cstl_dlist_verify_sorted_rev, &in,
                       CSTL_DLIST_FOREACH_DIR_REV);

    cstl_dlist_clear(&l, __test_cstl_dlist_free);
    ck_assert_uint_eq(cstl_dlist_size(&l), 0);
}
END_TEST

START_TEST(reverse)
{
    static const size_t n = 100;
    DECLARE_CSTL_DLIST(l, struct integer, ln);

    struct integer * in = NULL;

    __test__cstl_dlist_fill(&l, n);

    cstl_dlist_sort(&l, cmp_integer, NULL);
    cstl_dlist_reverse(&l);
    cstl_dlist_foreach(&l,
                       cstl_dlist_verify_sorted_rev, &in,
                       CSTL_DLIST_FOREACH_DIR_FWD);

    cstl_dlist_clear(&l, __test_cstl_dlist_free);
    ck_assert_uint_eq(cstl_dlist_size(&l), 0);
}
END_TEST

START_TEST(swap)
{
    DECLARE_CSTL_DLIST(l1, struct integer, ln);
    DECLARE_CSTL_DLIST(l2, struct integer, ln);

    __test__cstl_dlist_fill(&l1, 0);
    cstl_dlist_swap(&l1, &l2);
    ck_assert_int_eq(cstl_dlist_size(&l1), 0);
    ck_assert_int_eq(cstl_dlist_size(&l2), 0);
    cstl_dlist_clear(&l1, __test_cstl_dlist_free);
    cstl_dlist_clear(&l2, __test_cstl_dlist_free);

    __test__cstl_dlist_fill(&l1, 1);
    cstl_dlist_swap(&l1, &l2);
    ck_assert_int_eq(cstl_dlist_size(&l1), 0);
    ck_assert_int_eq(cstl_dlist_size(&l2), 1);
    cstl_dlist_clear(&l1, __test_cstl_dlist_free);
    cstl_dlist_clear(&l2, __test_cstl_dlist_free);

    __test__cstl_dlist_fill(&l1, 2);
    cstl_dlist_swap(&l1, &l2);
    ck_assert_int_eq(cstl_dlist_size(&l1), 0);
    ck_assert_int_eq(cstl_dlist_size(&l2), 2);
    cstl_dlist_clear(&l1, __test_cstl_dlist_free);
    cstl_dlist_clear(&l2, __test_cstl_dlist_free);

    __test__cstl_dlist_fill(&l1, 2);
    __test__cstl_dlist_fill(&l2, 3);
    cstl_dlist_swap(&l1, &l2);
    ck_assert_int_eq(cstl_dlist_size(&l1), 3);
    ck_assert_int_eq(cstl_dlist_size(&l2), 2);
    cstl_dlist_clear(&l1, __test_cstl_dlist_free);
    cstl_dlist_clear(&l2, __test_cstl_dlist_free);
}
END_TEST

Suite * dlist_suite(void)
{
    Suite * const s = suite_create("dlist");

    TCase * tc;

    tc = tcase_create("dlist");
    tcase_add_test(tc, simple);
    tcase_add_test(tc, fill);
    tcase_add_test(tc, concat);
    tcase_add_test(tc, sort);
    tcase_add_test(tc, reverse);
    tcase_add_test(tc, swap);
    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
