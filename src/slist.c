/*!
 * @file
 */

#include "cstl/slist.h"

#include <assert.h>

/*! @private */
static void * __cstl_slist_element(const struct cstl_slist * const s,
                                   const struct cstl_slist_node * const n)
{
    return (void *)((uintptr_t)n - s->off);
}

/*! @private */
static struct cstl_slist_node * __cstl_slist_node(
    const struct cstl_slist * const s, const void * const e)
{
    return (struct cstl_slist_node *)((uintptr_t)e + s->off);
}

/*! @private */
static void __cstl_slist_insert_after(struct cstl_slist * const sl,
                                      struct cstl_slist_node * const in,
                                      struct cstl_slist_node * const nn)
{
    assert(sl->t->n == NULL);
    nn->n = in->n;
    in->n = nn;

    if (sl->t == in) {
        sl->t = nn;
    }

    sl->count++;
    assert(sl->t->n == NULL);
}

/*! @private */
static struct cstl_slist_node * __cstl_slist_erase_after(
    struct cstl_slist * const sl, struct cstl_slist_node * const e)
{
    struct cstl_slist_node * const n = e->n;

    assert(sl->t->n == NULL);
    e->n = n->n;
    if (sl->t == n) {
        sl->t = e;
    }
    assert(sl->t->n == NULL);

    sl->count--;

    return n;
}

void cstl_slist_insert_after(struct cstl_slist * const sl,
                             void * const e, void * const n)
{
    __cstl_slist_insert_after(
        sl, __cstl_slist_node(sl, e), __cstl_slist_node(sl, n));
}

void * cstl_slist_erase_after(struct cstl_slist * const sl, void * const e)
{
    return __cstl_slist_element(
        sl, __cstl_slist_erase_after(sl, __cstl_slist_node(sl, e)));
}

void cstl_slist_push_front(struct cstl_slist * const sl, void * const e)
{
    __cstl_slist_insert_after(sl, &sl->h, __cstl_slist_node(sl, e));
}

void cstl_slist_push_back(struct cstl_slist * const sl, void * const e)
{
    __cstl_slist_insert_after(sl, sl->t, __cstl_slist_node(sl, e));
}

void * cstl_slist_pop_front(struct cstl_slist * const sl)
{
    return __cstl_slist_element(sl, __cstl_slist_erase_after(sl, &sl->h));
}

void * cstl_slist_front(const struct cstl_slist * const sl)
{
    if (sl->t == &sl->h) {
        return NULL;
    }
    return __cstl_slist_element(sl, sl->h.n);
}

void * cstl_slist_back(const struct cstl_slist * const sl)
{
    if (sl->t == &sl->h) {
        return NULL;
    }
    return __cstl_slist_element(sl, sl->t);
}

void cstl_slist_reverse(struct cstl_slist * const sl)
{
    if (cstl_slist_size(sl) > 1) {
        struct cstl_slist_node * const c = sl->h.n;

        /*
         * while there is a node after the current
         * node, splice that next node out and reinsert
         * it at the head of the list.
         */
        while (c->n != NULL) {
            struct cstl_slist_node * const n = c->n;

            c->n = n->n;
            n->n = sl->h.n;
            sl->h.n = n;
        }

        sl->t = c;
        assert(sl->t->n == NULL);
    }
}

void cstl_slist_concat(struct cstl_slist * const dst,
                       struct cstl_slist * const src)
{
    if (cstl_slist_size(src) > 0 && dst->off == src->off) {
        assert(dst->t->n == NULL);
        dst->t->n = src->h.n;
        dst->t = src->t;
        assert(dst->t->n == NULL);

        dst->count += src->count;

        cstl_slist_init(src, src->off);
    }
}

void cstl_slist_sort(struct cstl_slist * const sl,
                     cstl_compare_func_t * const cmp, void * const cmp_p)
{
    if (cstl_slist_size(sl) > 1) {
        struct cstl_slist _sl[2];
        struct cstl_slist_node * t;

        cstl_slist_init(&_sl[0], sl->off);
        cstl_slist_init(&_sl[1], sl->off);

        /* split the list in half */

        for (t = &sl->h;
             _sl[0].count < sl->count / 2;
             t = t->n, _sl[0].count++)
            ;

        _sl[0].h.n = sl->h.n;
        _sl[0].t = t;

        _sl[1].h.n = t->n;
        _sl[1].t = sl->t;
        t->n = NULL;

        _sl[1].count = sl->count - _sl[0].count;
        cstl_slist_init(sl, sl->off);

        /* sort the halves */
        cstl_slist_sort(&_sl[0], cmp, cmp_p);
        cstl_slist_sort(&_sl[1], cmp, cmp_p);

        /*
         * merge the two halves back together by
         * moving the lesser element from the front
         * of the two lists to the end of the output
         */
        while (cstl_slist_size(&_sl[0]) > 0
               && cstl_slist_size(&_sl[1]) > 0) {
            struct cstl_slist * l;

            if (cmp(__cstl_slist_element(&_sl[0], _sl[0].h.n),
                    __cstl_slist_element(&_sl[1], _sl[1].h.n),
                    cmp_p) <= 0) {
                l = &_sl[0];
            } else {
                l = &_sl[1];
            }

            __cstl_slist_insert_after(sl, sl->t,
                                      __cstl_slist_erase_after(l, &l->h));
        }

        /* concatenate whatever is left */
        if (cstl_slist_size(&_sl[0]) > 0) {
            cstl_slist_concat(sl, &_sl[0]);
        } else {
            cstl_slist_concat(sl, &_sl[1]);
        }
    }
}

int cstl_slist_foreach(struct cstl_slist * const sl,
                       cstl_visit_func_t * const visit, void * const p)
{
    struct cstl_slist_node * c = sl->h.n;
    int res = 0;

    while (c != NULL && res == 0) {
        struct cstl_slist_node * const n = c->n;
        res = visit(__cstl_slist_element(sl, c), p);
        c = n;
    }

    return res;
}

void cstl_slist_clear(struct cstl_slist * const sl,
                      cstl_xtor_func_t * const clr)
{
    struct cstl_slist_node * h;

    h = sl->h.n;
    while (h != NULL) {
        struct cstl_slist_node * const n = h->n;
        clr(__cstl_slist_element(sl, h), NULL);
        h = n;
    }

    cstl_slist_init(sl, sl->off);
}

void cstl_slist_swap(struct cstl_slist * const a, struct cstl_slist * const b)
{
    struct cstl_slist t;

    cstl_swap(a, b, &t, sizeof(t));

#ifndef NO_DOC
#define CSTL_SLIST_FIX_SWAP(SL)                 \
    do {                                        \
        if (SL->count == 0) {                   \
            SL->t = &SL->h;                     \
        }                                       \
    } while (0)

    CSTL_SLIST_FIX_SWAP(a);
    CSTL_SLIST_FIX_SWAP(b);

#undef CSTL_SLIST_FIX_SWAP
#endif
}

#ifdef __cfg_test__
// GCOV_EXCL_START
#include <check.h>
#include <stdlib.h>

struct integer {
    int v;
    struct cstl_slist_node sn;
};

static int cmp_integer(const void * const a, const void * const b,
                       void * const p)
{
    (void)p;
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

static void __test_cstl_slist_free(void * const p, void * const x)
{
    (void)x;
    free(p);
}

static void __test__cstl_slist_fill(
    struct cstl_slist * const sl, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * in = malloc(sizeof(*in));

        in->v = rand() % n;
        cstl_slist_push_front(sl, in);
    }

    ck_assert_uint_eq(n, cstl_slist_size(sl));
}

START_TEST(simple)
{
    DECLARE_CSTL_SLIST(l, struct integer, sn);
    struct integer a, b, c;

    a.v = 0; b.v = 1; c.v = 2;

    ck_assert_int_eq(cstl_slist_size(&l), 0);
    ck_assert_ptr_null(cstl_slist_front(&l));
    ck_assert_ptr_null(cstl_slist_back(&l));

    cstl_slist_push_front(&l, &a);
    ck_assert_int_eq(cstl_slist_size(&l), 1);
    ck_assert_ptr_eq(cstl_slist_front(&l), &a);
    ck_assert_ptr_eq(cstl_slist_back(&l), &a);

    cstl_slist_insert_after(&l, &a, &b);
    ck_assert_int_eq(cstl_slist_size(&l), 2);
    ck_assert_ptr_eq(cstl_slist_front(&l), &a);
    ck_assert_ptr_eq(cstl_slist_back(&l), &b);

    cstl_slist_push_back(&l, &c);
    ck_assert_int_eq(cstl_slist_size(&l), 3);
    ck_assert_ptr_eq(cstl_slist_front(&l), &a);
    ck_assert_ptr_eq(cstl_slist_back(&l), &c);

    cstl_slist_erase_after(&l, &a);
    ck_assert_int_eq(cstl_slist_size(&l), 2);

    ck_assert_ptr_eq(cstl_slist_pop_front(&l), &a);
    ck_assert_int_eq(cstl_slist_size(&l), 1);

    ck_assert_ptr_eq(cstl_slist_pop_front(&l), &c);
    ck_assert_int_eq(cstl_slist_size(&l), 0);
}
END_TEST

START_TEST(fill)
{
    static const size_t n = 100;
    DECLARE_CSTL_SLIST(sl, struct integer, sn);

    __test__cstl_slist_fill(&sl, n);

    cstl_slist_clear(&sl, __test_cstl_slist_free);
    ck_assert_uint_eq(cstl_slist_size(&sl), 0);
}
END_TEST

START_TEST(concat)
{
    static const size_t n = 4;
    DECLARE_CSTL_SLIST(l1, struct integer, sn);
    DECLARE_CSTL_SLIST(l2, struct integer, sn);

    __test__cstl_slist_fill(&l1, n);
    __test__cstl_slist_fill(&l2, n);

    cstl_slist_concat(&l1, &l2);
    ck_assert_uint_eq(cstl_slist_size(&l1), 2 * n);
    ck_assert_uint_eq(cstl_slist_size(&l2), 0);

    cstl_slist_clear(&l1, __test_cstl_slist_free);
    cstl_slist_clear(&l2, __test_cstl_slist_free);
}
END_TEST

static int cstl_slist_verify_sorted(void * const e, void * const p)
{
    struct integer ** in = p;

    if (*in != NULL) {
        ck_assert_int_ge(((struct integer *)e)->v, (*in)->v);
    }

    *in = e;

    return 0;
}

START_TEST(sort)
{
    static const size_t n = 100;
    DECLARE_CSTL_SLIST(l, struct integer, sn);

    struct integer * in = NULL;

    __test__cstl_slist_fill(&l, n);

    cstl_slist_sort(&l, cmp_integer, NULL);
    ck_assert_uint_eq(n, cstl_slist_size(&l));
    cstl_slist_foreach(&l, cstl_slist_verify_sorted, &in);

    cstl_slist_clear(&l, __test_cstl_slist_free);
    ck_assert_uint_eq(cstl_slist_size(&l), 0);
}
END_TEST

static int cstl_slist_verify_sorted_rev(void * const e, void * const p)
{
    struct integer ** in = p;

    if (*in != NULL) {
        ck_assert_int_le(((struct integer *)e)->v, (*in)->v);
    }

    *in = e;

    return 0;
}

START_TEST(reverse)
{
    static const size_t n = 100;
    DECLARE_CSTL_SLIST(l, struct integer, sn);

    struct integer * in = NULL;

    __test__cstl_slist_fill(&l, n);

    cstl_slist_sort(&l, cmp_integer, NULL);
    cstl_slist_reverse(&l);
    cstl_slist_foreach(&l, cstl_slist_verify_sorted_rev, &in);

    cstl_slist_clear(&l, __test_cstl_slist_free);
    ck_assert_uint_eq(cstl_slist_size(&l), 0);
}
END_TEST

START_TEST(swap)
{
    DECLARE_CSTL_SLIST(l1, struct integer, sn);
    DECLARE_CSTL_SLIST(l2, struct integer, sn);

    __test__cstl_slist_fill(&l1, 0);
    cstl_slist_swap(&l1, &l2);
    ck_assert_int_eq(cstl_slist_size(&l1), 0);
    ck_assert_int_eq(cstl_slist_size(&l2), 0);
    cstl_slist_clear(&l1, __test_cstl_slist_free);
    cstl_slist_clear(&l2, __test_cstl_slist_free);

    __test__cstl_slist_fill(&l1, 1);
    cstl_slist_swap(&l1, &l2);
    ck_assert_int_eq(cstl_slist_size(&l1), 0);
    ck_assert_int_eq(cstl_slist_size(&l2), 1);
    cstl_slist_clear(&l1, __test_cstl_slist_free);
    cstl_slist_clear(&l2, __test_cstl_slist_free);

    __test__cstl_slist_fill(&l1, 2);
    cstl_slist_swap(&l1, &l2);
    ck_assert_int_eq(cstl_slist_size(&l1), 0);
    ck_assert_int_eq(cstl_slist_size(&l2), 2);
    cstl_slist_clear(&l1, __test_cstl_slist_free);
    cstl_slist_clear(&l2, __test_cstl_slist_free);

    __test__cstl_slist_fill(&l1, 2);
    __test__cstl_slist_fill(&l2, 3);
    cstl_slist_swap(&l1, &l2);
    ck_assert_int_eq(cstl_slist_size(&l1), 3);
    ck_assert_int_eq(cstl_slist_size(&l2), 2);
    cstl_slist_clear(&l1, __test_cstl_slist_free);
    cstl_slist_clear(&l2, __test_cstl_slist_free);
}
END_TEST

Suite * slist_suite(void)
{
    Suite * const s = suite_create("slist");

    TCase * tc;

    tc = tcase_create("slist");
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
