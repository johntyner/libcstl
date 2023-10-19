#include "slist.h"

static void * __slist_element(const struct slist * const s,
                              struct slist_node * const n)
{
    return (void *)((uintptr_t)n - s->off);
}

static struct slist_node * __slist_node(const struct slist * const s,
                                        void * const e)
{
    return (struct slist_node *)((uintptr_t)e + s->off);
}

static void __slist_insert_after(struct slist * const sl,
                                 struct slist_node * const in,
                                 struct slist_node * const nn)
{
    nn->n = in->n;
    in->n = nn;

    sl->count++;
}

struct slist_node * __slist_erase_after(struct slist * const sl,
                                        struct slist_node * const e)
{
    struct slist_node * const n = e->n;

    e->n = n->n;
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

void * slist_pop_front(struct slist * const sl)
{
    return __slist_element(sl, __slist_erase_after(sl, &sl->h));
}

void * slist_front(const struct slist * const sl)
{
    return __slist_element(sl, sl->h.n);
}

void slist_reverse(struct slist * const sl)
{
    if (sl->h.n != NULL) {
        struct slist_node * const c = sl->h.n;

        while (c->n) {
            struct slist_node * const n = c->n;

            c->n = n->n;
            n->n = sl->h.n;
            sl->h.n = n;
        }
    }
}

void slist_concat(struct slist * const dst, struct slist * const src)
{
    if (slist_size(src) > 0) {
        struct slist_node * t;

        for (t = &dst->h; t->n != NULL; t = t->n)
            ;

        t->n = src->h.n;
        dst->count += src->count;

        src->h.n = NULL;
        src->count = 0;
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

        slist_concat(&_sl[1], sl);

        for (t = &_sl[1].h;
             _sl[0].count < _sl[1].count / 2;
             t = t->n, _sl[0].count++)
            ;

        _sl[0].h.n = _sl[1].h.n;
        _sl[1].h.n = t->n;
        t->n = NULL;
        _sl[1].count -= _sl[0].count;

        slist_sort(&_sl[0], cmp, cmp_p);
        slist_sort(&_sl[1], cmp, cmp_p);

        t = &sl->h;
        while (slist_size(&_sl[0]) > 0
               && slist_size(&_sl[1]) > 0) {
            struct slist * l;
            struct slist_node * n;

            if (cmp(__slist_element(&_sl[0], _sl[0].h.n),
                    __slist_element(&_sl[1], _sl[1].h.n),
                    cmp_p) <= 0) {
                l = &_sl[0];
            } else {
                l = &_sl[1];
            }

            n = __slist_erase_after(l, &l->h);
            __slist_insert_after(sl, t, n);
            t = n;
        }

        if (slist_size(&_sl[0]) > 0) {
            slist_concat(sl, &_sl[0]);
        } else {
            slist_concat(sl, &_sl[1]);
        }
    }
}

int __slist_foreach(struct slist * const sl, struct slist_node * c,
                    cstl_visit_func_t * const visit, void * const p)
{
    int res = 0;

    while (c != NULL && res == 0) {
        struct slist_node * const n = c->n;
        res = visit(__slist_element(sl, c), p);
        c = n;
    }

    return res;
}

int slist_foreach(struct slist * const sl,
                  cstl_visit_func_t * const visit, void * const p)
{
    return __slist_foreach(sl, sl->h.n, visit, p);
}

struct slist_clear_priv
{
    cstl_clear_func_t * clr;
};

int __slist_clear(void * const e, void * const p)
{
    struct slist_clear_priv * const scp = p;
    scp->clr(e, NULL);
    return 0;
}

void slist_clear(struct slist * const sl, cstl_clear_func_t * const clr)
{
    struct slist_clear_priv scp;

    scp.clr = clr;
    slist_foreach(sl, __slist_clear, &scp);

    sl->h.n = NULL;
    sl->count = 0;
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
    cstl_free(p);
}

static void __test__slist_fill(struct slist * const sl, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * in = cstl_malloc(sizeof(*in));

        in->v = rand() % n;
        slist_push_front(sl, in);
    }

    ck_assert_uint_eq(n, slist_size(sl));
}

START_TEST(fill)
{
    static const size_t n = 100;
    struct slist sl;

    SLIST_INIT(&sl, struct integer, sn);

    __test__slist_fill(&sl, n);

    slist_clear(&sl, __test_slist_free);
    ck_assert_uint_eq(slist_size(&sl), 0);
}
END_TEST

START_TEST(concat)
{
    static const size_t n = 4;
    struct slist l1, l2;

    SLIST_INIT(&l1, struct integer, sn);
    SLIST_INIT(&l2, struct integer, sn);

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
    struct slist l;

    struct integer * in = NULL;

    SLIST_INIT(&l, struct integer, sn);

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
    struct slist l;

    struct integer * in = NULL;

    SLIST_INIT(&l, struct integer, sn);

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
    struct slist l1, l2;

    SLIST_INIT(&l1, struct integer, sn);
    SLIST_INIT(&l2, struct integer, sn);

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
