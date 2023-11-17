/*!
 * @file
 */

#include "cstl/vector.h"
#include "cstl/array.h"

#include <stdlib.h>

/*! @private */
static void * __cstl_vector_at(
    const struct cstl_vector * const v, const size_t i)
{
    return (void *)((uintptr_t)v->elem.base + i * v->elem.size);
}

const void * cstl_vector_at_const(
    const struct cstl_vector * const v, const size_t i)
{
    if (i >= v->count) {
        CSTL_ABORT();
    }

    return __cstl_vector_at(v, i);
}

void * cstl_vector_at(struct cstl_vector * const v, const size_t i)
{
    return (void *)cstl_vector_at_const(v, i);
}

/*! @private */
static void cstl_vector_set_capacity(
    struct cstl_vector * const v, const size_t sz)
{
    if (sz < v->count) {
        /*
         * this shouldn't be able to be triggered by the
         * api. it's here to catch bugs internally in the
         * vector code
         */
        CSTL_ABORT(); // GCOV_EXCL_LINE
    } else {
        /*
         * the vector always (quietly) stores space for one extra
         * element at the end to use as scratch space for exchanging
         * elements during sort and reverse operations
         */
        void * const e = realloc(v->elem.base, (sz + 1) * v->elem.size);
        if (e != NULL) {
            v->elem.base = e;
            v->cap = sz;
        }
    }
}

void cstl_vector_reserve(struct cstl_vector * const v, const size_t sz)
{
    if (sz > v->cap) {
        cstl_vector_set_capacity(v, sz);
    }
}

void cstl_vector_shrink_to_fit(struct cstl_vector * const v)
{
    if (v->cap > v->count) {
        cstl_vector_set_capacity(v, v->count);
    }
}

void cstl_vector_resize(struct cstl_vector * const v, const size_t sz)
{
    void * const priv = v->elem.xtor.priv;
    cstl_xtor_func_t * xtor = NULL;

    cstl_vector_reserve(v, sz);
    if (v->cap < sz) {
        /*
         * this is designed to catch a failure to allocate
         * memory. it represents a runtime error that the
         * caller is not expected to ever have to deal with
         */
        CSTL_ABORT(); // GCOV_EXCL_LINE
    }

    if (v->count < sz) {
        xtor = v->elem.xtor.cons;
    } else if (v->count > sz) {
        xtor = v->elem.xtor.dest;
    }

    if (xtor == NULL) {
        v->count = sz;
    } else if (v->count < sz) {
        do {
            xtor(__cstl_vector_at(v, v->count++), priv);
        } while (v->count < sz);
    } else if (v->count > sz) {
        do {
            xtor(__cstl_vector_at(v, --v->count), priv);
        } while (v->count > sz);
    }
}

void cstl_vector_clear(struct cstl_vector * const v)
{
    cstl_vector_resize(v, 0);
    free(v->elem.base);

    v->elem.base = NULL;
    v->cap = 0;
}

void __cstl_vector_sort(struct cstl_vector * const v,
                        cstl_compare_func_t * const cmp, void * const priv,
                        cstl_swap_func_t * const swap,
                        const cstl_sort_algorithm_t algo)
{
    cstl_raw_array_sort(
        v->elem.base, v->count, v->elem.size,
        cmp, priv,
        swap, __cstl_vector_at(v, v->cap),
        algo);
}

ssize_t cstl_vector_search(const struct cstl_vector * const v,
                           const void * const e,
                           cstl_compare_func_t * const cmp,
                           void * const priv)
{
    return cstl_raw_array_search(v->elem.base,
                                 v->count, v->elem.size,
                                 e,
                                 cmp, priv);
}

ssize_t cstl_vector_find(const struct cstl_vector * const v,
                         const void * const e,
                         cstl_compare_func_t * const cmp,
                         void * const priv)
{
    return cstl_raw_array_find(v->elem.base,
                               v->count, v->elem.size,
                               e,
                               cmp, priv);
}

void __cstl_vector_reverse(struct cstl_vector * const v,
                           cstl_swap_func_t * const swap)
{
    cstl_raw_array_reverse(v->elem.base,
                           v->count, v->elem.size,
                           swap, __cstl_vector_at(v, v->cap));
}

void cstl_vector_swap(struct cstl_vector * const a,
                      struct cstl_vector * const b)
{
    struct cstl_vector t;
    cstl_swap(a, b, &t, sizeof(t));
}

#ifdef __cstl_cfg_test__
// GCOV_EXCL_START
#include <check.h>

#include <stdlib.h>

static int int_cmp(const void * const a, const void * const b,
                   void * const p)
{
    (void)p;
    return *(int *)a - *(int *)b;
}

START_TEST(sort)
{
    static size_t n = 71;
    static const cstl_sort_algorithm_t algo[] = {
        CSTL_SORT_ALGORITHM_QUICK,
        CSTL_SORT_ALGORITHM_QUICK_R,
        CSTL_SORT_ALGORITHM_QUICK_M,
        CSTL_SORT_ALGORITHM_HEAP,
        /*
         * a wildly wrong enumeration to ensure that the
         * vector still gets sorted
         */
        2897234,
    };

    DECLARE_CSTL_VECTOR(v, int);
    unsigned int i;

    cstl_vector_resize(&v, n);

    for (i = 0; i < sizeof(algo) / sizeof(*algo); i++) {
        unsigned int j;

        for (j = 0; j < n; j++) {
            *(int *)cstl_vector_at(&v, j) = rand() % n;
        }
        __cstl_vector_sort(&v, int_cmp, NULL, cstl_swap, algo[i]);
        for (j = 1; j < n; j++) {
            ck_assert_int_ge(*(int *)cstl_vector_at(&v, j),
                             *(int *)cstl_vector_at(&v, j - 1));
        }
    }

    cstl_vector_clear(&v);
}
END_TEST

START_TEST(invalid_access)
{
    DECLARE_CSTL_VECTOR(v, int);

    cstl_vector_resize(&v, 5);

    ck_assert_signal(SIGABRT, cstl_vector_at(&v, -1));
    ck_assert_signal(SIGABRT, cstl_vector_at(&v, 5));

    cstl_vector_clear(&v);
}
END_TEST

START_TEST(search)
{
    static size_t n = 63;

    DECLARE_CSTL_VECTOR(v, int);
    unsigned int i;

    cstl_vector_resize(&v, n);
    for (i = 0; i < n; i++) {
        *(int *)cstl_vector_at(&v, i) = i;
    }

    for (i = 0; i < n; i++) {
        ck_assert_int_eq(cstl_vector_search(&v, &i, int_cmp, NULL), i);
    }
    ck_assert_int_eq(cstl_vector_search(&v, &i, int_cmp, NULL), -1);

    for (i = 0; i < n; i++) {
        ck_assert_int_eq(cstl_vector_find(&v, &i, int_cmp, NULL), i);
    }
    ck_assert_int_eq(cstl_vector_find(&v, &i, int_cmp, NULL), -1);

    cstl_vector_clear(&v);
}
END_TEST

START_TEST(reverse)
{
    static size_t n = 27;

    DECLARE_CSTL_VECTOR(v, int);
    unsigned int i;

    cstl_vector_resize(&v, n);
    for (i = 0; i < n; i++) {
        *(int *)cstl_vector_at(&v, i) = i;
    }

    cstl_vector_reverse(&v);
    for (i = 1; i < n; i++) {
        ck_assert_int_le(*(int *)cstl_vector_at(&v, i),
                         *(int *)cstl_vector_at(&v, i - 1));
    }

    cstl_vector_clear(&v);
}
END_TEST

static void int_cons(void * const i, void * const p)
{
    (void)p;

    *(int *)i = 0;
}

static void int_dest(void * const i, void * const p)
{
    (void)p;

    *(int *)i = -1;
}

START_TEST(complex)
{
    struct cstl_vector v;
    int i;

    cstl_vector_init_complex(&v, sizeof(int),
                             int_cons, int_dest, NULL);

    cstl_vector_resize(&v, 10);
    for (i = 0; i < (int)cstl_vector_size(&v); i++) {
        ck_assert_int_eq(*(int *)cstl_vector_at(&v, i), 0);
    }
    ck_assert_int_eq(i, 10);
    for (; i < (int)cstl_vector_capacity(&v); i++) {
        ck_assert_int_eq(*(int *)__cstl_vector_at(&v, i), -1);
    }
    ck_assert_int_eq(i, 10);

    cstl_vector_resize(&v, 3);
    for (i = 0; i < (int)cstl_vector_size(&v); i++) {
        ck_assert_int_eq(*(int *)cstl_vector_at(&v, i), 0);
    }
    ck_assert_int_eq(i, 3);
    for (; i < (int)cstl_vector_capacity(&v); i++) {
        ck_assert_int_eq(*(int *)__cstl_vector_at(&v, i), -1);
    }
    ck_assert_int_eq(i, 10);

    cstl_vector_resize(&v, 6);
    for (i = 0; i < (int)cstl_vector_size(&v); i++) {
        ck_assert_int_eq(*(int *)cstl_vector_at(&v, i), 0);
    }
    ck_assert_int_eq(i, 6);
    for (; i < (int)cstl_vector_capacity(&v); i++) {
        ck_assert_int_eq(*(int *)__cstl_vector_at(&v, i), -1);
    }
    ck_assert_int_eq(i, 10);

    ck_assert_int_ne(cstl_vector_size(&v), cstl_vector_capacity(&v));
    cstl_vector_shrink_to_fit(&v);
    ck_assert_int_eq(cstl_vector_size(&v), cstl_vector_capacity(&v));

    cstl_vector_clear(&v);
}
END_TEST

Suite * vector_suite(void)
{
    Suite * const s = suite_create("vector");

    TCase * tc;

    tc = tcase_create("vector");
    tcase_add_test(tc, invalid_access);
    tcase_add_test(tc, sort);
    tcase_add_test(tc, search);
    tcase_add_test(tc, reverse);
    tcase_add_test(tc, complex);
    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
