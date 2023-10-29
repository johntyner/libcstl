/*!
 * @file
 */

#include "slist.h"

#include <assert.h>

/*! @private */
static void * __slist_element(const struct slist * const s,
                              struct slist_node * const n)
{
    return (void *)((uintptr_t)n - s->off);
}

/*! @private */
static struct slist_node * __slist_node(const struct slist * const s,
                                        void * const e)
{
    return (struct slist_node *)((uintptr_t)e + s->off);
}

/*! @private */
static void __slist_insert_after(struct slist * const sl,
                                 struct slist_node * const in,
                                 struct slist_node * const nn)
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
static struct slist_node * __slist_erase_after(struct slist * const sl,
                                               struct slist_node * const e)
{
    struct slist_node * const n = e->n;

    assert(sl->t->n == NULL);
    e->n = n->n;
    if (sl->t == n) {
        sl->t = e;
    }
    assert(sl->t->n == NULL);

    sl->count--;

    return n;
}

void slist_insert_after(struct slist * const sl,
                        void * const e, void * const n)
{
    __slist_insert_after(sl, __slist_node(sl, e), __slist_node(sl, n));

}

void * slist_erase_after(struct slist * const sl, void * const e)
{
    return __slist_element(sl, __slist_erase_after(sl, __slist_node(sl, e)));
}

void slist_push_front(struct slist * const sl, void * const e)
{
    __slist_insert_after(sl, &sl->h, __slist_node(sl, e));
}

void slist_push_back(struct slist * const sl, void * const e)
{
    __slist_insert_after(sl, sl->t, __slist_node(sl, e));
}

void * slist_pop_front(struct slist * const sl)
{
    return __slist_element(sl, __slist_erase_after(sl, &sl->h));
}

void * slist_front(const struct slist * const sl)
{
    if (sl->t == &sl->h) {
        return NULL;
    }
    return __slist_element(sl, sl->h.n);
}

void * slist_back(const struct slist * const sl)
{
    if (sl->t == &sl->h) {
        return NULL;
    }
    return __slist_element(sl, sl->t);
}

void slist_reverse(struct slist * const sl)
{
    if (slist_size(sl) > 1) {
        struct slist_node * const c = sl->h.n;

        /*
         * while there is a node after the current
         * node, splice that next node out and reinsert
         * it at the head of the list.
         */
        while (c->n != NULL) {
            struct slist_node * const n = c->n;

            c->n = n->n;
            n->n = sl->h.n;
            sl->h.n = n;
        }

        sl->t = c;
        assert(sl->t->n == NULL);
    }
}

void slist_concat(struct slist * const dst, struct slist * const src)
{
    if (slist_size(src) > 0
        && dst->off == src->off) {
        assert(dst->t->n == NULL);
        dst->t->n = src->h.n;
        dst->t = src->t;
        assert(dst->t->n == NULL);

        dst->count += src->count;

        slist_init(src, src->off);
    }
}

void slist_sort(struct slist * const sl,
                cstl_compare_func_t * const cmp, void * const cmp_p)
{
    if (slist_size(sl) > 1) {
        struct slist _sl[2];
        struct slist_node * t;

        slist_init(&_sl[0], sl->off);
        slist_init(&_sl[1], sl->off);

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
        slist_init(sl, sl->off);

        /* sort the halves */
        slist_sort(&_sl[0], cmp, cmp_p);
        slist_sort(&_sl[1], cmp, cmp_p);

        /*
         * merge the two halves back together by
         * moving the lesser element from the front
         * of the two lists to the end of the output
         */
        while (slist_size(&_sl[0]) > 0
               && slist_size(&_sl[1]) > 0) {
            struct slist * l;

            if (cmp(__slist_element(&_sl[0], _sl[0].h.n),
                    __slist_element(&_sl[1], _sl[1].h.n),
                    cmp_p) <= 0) {
                l = &_sl[0];
            } else {
                l = &_sl[1];
            }

            __slist_insert_after(sl, sl->t,
                                 __slist_erase_after(l, &l->h));
        }

        /* concatenate whatever is left */
        if (slist_size(&_sl[0]) > 0) {
            slist_concat(sl, &_sl[0]);
        } else {
            slist_concat(sl, &_sl[1]);
        }
    }
}

int slist_foreach(struct slist * const sl,
                  cstl_visit_func_t * const visit, void * const p)
{
    struct slist_node * c = sl->h.n;
    int res = 0;

    while (c != NULL && res == 0) {
        struct slist_node * const n = c->n;
        res = visit(__slist_element(sl, c), p);
        c = n;
    }

    return res;
}

void slist_clear(struct slist * const sl, cstl_clear_func_t * const clr)
{
    struct slist_node * h;

    h = sl->h.n;
    while (h != NULL) {
        struct slist_node * const n = h->n;
        clr(__slist_element(sl, h), NULL);
        h = n;
    }

    slist_init(sl, sl->off);
}

void slist_swap(struct slist * const a, struct slist * const b)
{
    struct slist t;

    cstl_swap(a, b, &t, sizeof(t));

#ifndef NO_DOC
#define SLIST_FIX_SWAP(SL)                      \
    do {                                        \
        if (SL->count == 0) {                   \
            SL->t = &SL->h;                     \
        }                                       \
    } while (0)

    SLIST_FIX_SWAP(a);
    SLIST_FIX_SWAP(b);

#undef SLIST_FIX_SWAP
#endif
}

#ifdef __cfg_test__
#include <check.h>
#include <stdlib.h>

struct integer {
    int v;
    struct slist_node sn;
};

static int cmp_integer(const void * const a, const void * const b,
                       void * const p)
{
    (void)p;
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

static void __test_slist_free(void * const p, void * const x)
{
    (void)x;
    free(p);
}

static void __test__slist_fill(struct slist * const sl, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * in = malloc(sizeof(*in));

        in->v = rand() % n;
        slist_push_front(sl, in);
    }

    ck_assert_uint_eq(n, slist_size(sl));
}

START_TEST(fill)
{
    static const size_t n = 100;
    DECLARE_SLIST(sl, struct integer, sn);

    __test__slist_fill(&sl, n);

    slist_clear(&sl, __test_slist_free);
    ck_assert_uint_eq(slist_size(&sl), 0);
}
END_TEST

START_TEST(concat)
{
    static const size_t n = 4;
    DECLARE_SLIST(l1, struct integer, sn);
    DECLARE_SLIST(l2, struct integer, sn);

    __test__slist_fill(&l1, n);
    __test__slist_fill(&l2, n);

    slist_concat(&l1, &l2);
    ck_assert_uint_eq(slist_size(&l1), 2 * n);
    ck_assert_uint_eq(slist_size(&l2), 0);

    slist_clear(&l1, __test_slist_free);
    slist_clear(&l2, __test_slist_free);
}
END_TEST

static int slist_verify_sorted(void * const e, void * const p)
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
    DECLARE_SLIST(l, struct integer, sn);

    struct integer * in = NULL;

    __test__slist_fill(&l, n);

    slist_sort(&l, cmp_integer, NULL);
    ck_assert_uint_eq(n, slist_size(&l));
    slist_foreach(&l, slist_verify_sorted, &in);

    slist_clear(&l, __test_slist_free);
    ck_assert_uint_eq(slist_size(&l), 0);
}
END_TEST

static int slist_verify_sorted_rev(void * const e, void * const p)
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
    DECLARE_SLIST(l, struct integer, sn);

    struct integer * in = NULL;

    __test__slist_fill(&l, n);

    slist_sort(&l, cmp_integer, NULL);
    slist_reverse(&l);
    slist_foreach(&l, slist_verify_sorted_rev, &in);

    slist_clear(&l, __test_slist_free);
    ck_assert_uint_eq(slist_size(&l), 0);
}
END_TEST

START_TEST(swap)
{
    DECLARE_SLIST(l1, struct integer, sn);
    DECLARE_SLIST(l2, struct integer, sn);

    __test__slist_fill(&l1, 0);
    slist_swap(&l1, &l2);
    ck_assert_int_eq(slist_size(&l1), 0);
    ck_assert_int_eq(slist_size(&l2), 0);
    slist_clear(&l1, __test_slist_free);
    slist_clear(&l2, __test_slist_free);

    __test__slist_fill(&l1, 1);
    slist_swap(&l1, &l2);
    ck_assert_int_eq(slist_size(&l1), 0);
    ck_assert_int_eq(slist_size(&l2), 1);
    slist_clear(&l1, __test_slist_free);
    slist_clear(&l2, __test_slist_free);

    __test__slist_fill(&l1, 2);
    slist_swap(&l1, &l2);
    ck_assert_int_eq(slist_size(&l1), 0);
    ck_assert_int_eq(slist_size(&l2), 2);
    slist_clear(&l1, __test_slist_free);
    slist_clear(&l2, __test_slist_free);

    __test__slist_fill(&l1, 2);
    __test__slist_fill(&l2, 3);
    slist_swap(&l1, &l2);
    ck_assert_int_eq(slist_size(&l1), 3);
    ck_assert_int_eq(slist_size(&l2), 2);
    slist_clear(&l1, __test_slist_free);
    slist_clear(&l2, __test_slist_free);
}
END_TEST

Suite * slist_suite(void)
{
    Suite * const s = suite_create("slist");

    TCase * tc;

    tc = tcase_create("slist");
    tcase_add_test(tc, fill);
    tcase_add_test(tc, concat);
    tcase_add_test(tc, sort);
    tcase_add_test(tc, reverse);
    tcase_add_test(tc, swap);
    suite_add_tcase(s, tc);

    return s;
}

#endif
