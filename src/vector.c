/*!
 * @file
 */

#include "cstl/vector.h"

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
        abort();
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
    /*
     * the capacity of an externally
     * allocated vector can't be changed
     */
    if (!v->elem.ext) {
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
        abort();
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
    if (!v->elem.ext) {
        free(v->elem.base);

        v->elem.base = NULL;
        v->cap = 0;
    }
}

/*! @private */
static size_t cstl_vector_qsort_p(struct cstl_vector * const v,
                                  size_t i, size_t j, const size_t p,
                                  cstl_compare_func_t * const cmp,
                                  void * const cmp_p,
                                  void * const t)
{
    const void * x = __cstl_vector_at(v, p);

    for (i--, j++;;) {
        void * a, * b;

        do {
            i++;
            a = __cstl_vector_at(v, i);
        } while (cmp(x, a, cmp_p) > 0);

        do {
            j--;
            b = __cstl_vector_at(v, j);
        } while (cmp(x, b, cmp_p) < 0);

        if (i >= j) {
            break;
        }

        if (x == a) {
            x = b;
        } else if (x == b) {
            x = a;
        }

        cstl_swap(a, b, t, v->elem.size);
    }

    return j;
}

/*! @private */
static void cstl_vector_qsort(struct cstl_vector * const v,
                              const size_t f, const size_t l,
                              cstl_compare_func_t * const cmp,
                              void * const cmp_p,
                              void * const tmp,
                              const int r)
{
    if (f < l) {
        size_t p = f;

        if (r != 0) {
            p = f + (rand() % (l - f + 1));
        }

        const size_t m = cstl_vector_qsort_p(v, f, l, p, cmp, cmp_p, tmp);
        cstl_vector_qsort(v, f, m, cmp, cmp_p, tmp, r);
        cstl_vector_qsort(v, m + 1, l, cmp, cmp_p, tmp, r);
    }
}

/*! @private */
static void cstl_vector_hsort_b(struct cstl_vector * const v, const size_t sz,
                                const unsigned int i,
                                cstl_compare_func_t * const cmp,
                                void * const cmp_p,
                                void * const tmp)
{
    const unsigned int l = 2 * i;
    const unsigned int r = l + 1;

    unsigned int n;

    n = i;
    if (l < sz
        && cmp(__cstl_vector_at(v, l), __cstl_vector_at(v, i), cmp_p) > 0) {
        n = l;
    }
    if (r < sz
        && cmp(__cstl_vector_at(v, r), __cstl_vector_at(v, n), cmp_p) > 0) {
        n = r;
    }

    if (n != i) {
        cstl_swap(__cstl_vector_at(v, i),
                  __cstl_vector_at(v, n),
                  tmp,
                  v->elem.size);
        cstl_vector_hsort_b(v, sz, n, cmp, cmp_p, tmp);
    }
}

/*! @private */
static void cstl_vector_hsort(struct cstl_vector * const v,
                              cstl_compare_func_t * const cmp,
                              void * const cmp_p,
                              void * const tmp)
{
    unsigned int i;

    for (i = v->count / 2; i > 0; i--) {
        cstl_vector_hsort_b(v, v->count, i - 1, cmp, cmp_p, tmp);
    }

    for (i = v->count - 1; i > 0; i--) {
        cstl_swap(
            __cstl_vector_at(v, 0),
            __cstl_vector_at(v, i),
            tmp,
            v->elem.size);
        cstl_vector_hsort_b(v, i, 0, cmp, cmp_p, tmp);
    }
}

void cstl_vector_sort(struct cstl_vector * const v,
                      cstl_compare_func_t * const cmp, void * const priv,
                      const cstl_vector_sort_algorithm_t algo)
{
    if (v->count > 1) {
        void * const tmp = __cstl_vector_at(v, v->cap);

        switch (algo) {
        case CSTL_VECTOR_SORT_ALGORITHM_QUICK:
            /* fallthrough */
        case CSTL_VECTOR_SORT_ALGORITHM_QUICK_R:
            cstl_vector_qsort(v, 0, v->count - 1, cmp, priv, tmp,
                              algo == CSTL_VECTOR_SORT_ALGORITHM_QUICK_R);
            break;
        case CSTL_VECTOR_SORT_ALGORITHM_HEAP:
            cstl_vector_hsort(v, cmp, priv, tmp);
            break;
        }
    }
}

ssize_t cstl_vector_search(const struct cstl_vector * const v,
                           const void * const e,
                           cstl_compare_func_t * const cmp,
                           void * const priv)
{
    if (v->count > 0) {
        unsigned int i, j;

        for (i = 0, j = v->count - 1; i <= j;) {
            const unsigned int n = (i + j) / 2;
            const int eq = cmp(e, __cstl_vector_at(v, n), priv);

            if (eq == 0) {
                return n;
            } else if (eq < 0) {
                j = n - 1;
            } else {
                i = n + 1;
            }
        }
    }

    return -1;
}

ssize_t cstl_vector_find(const struct cstl_vector * const v,
                         const void * const e,
                         cstl_compare_func_t * const cmp,
                         void * const priv)
{
    unsigned int i;

    for (i = 0; i < v->count; i++) {
        if (cmp(e, __cstl_vector_at(v, i), priv) == 0) {
            return i;
        }
    }

    return -1;
}

void cstl_vector_reverse(struct cstl_vector * const v)
{
    if (v->count > 1) {
        unsigned int i, j;

        for (i = 0, j = v->count - 1; i < j; i++, j--) {
            cstl_swap(__cstl_vector_at(v, i), __cstl_vector_at(v, j),
                      __cstl_vector_at(v, v->cap),
                      v->elem.size);
        }
    }
}

void cstl_vector_swap(struct cstl_vector * const a,
                      struct cstl_vector * const b)
{
    struct cstl_vector t;
    cstl_swap(a, b, &t, sizeof(t));
}

#ifdef __cfg_test__
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

    DECLARE_CSTL_VECTOR(v, int);
    unsigned int i;

    cstl_vector_resize(&v, n);

    for (i = 0; i < n; i++) {
        *(int *)cstl_vector_at(&v, i) = rand() % n;
    }
    cstl_vector_sort(&v, int_cmp, NULL, CSTL_VECTOR_SORT_ALGORITHM_QUICK);
    for (i = 1; i < n; i++) {
        ck_assert_int_ge(*(int *)cstl_vector_at(&v, i),
                         *(int *)cstl_vector_at(&v, i - 1));
    }

    for (i = 0; i < n; i++) {
        *(int *)cstl_vector_at(&v, i) = rand() % n;
    }
    cstl_vector_sort(&v, int_cmp, NULL, CSTL_VECTOR_SORT_ALGORITHM_QUICK_R);
    for (i = 1; i < n; i++) {
        ck_assert_int_ge(*(int *)cstl_vector_at(&v, i),
                         *(int *)cstl_vector_at(&v, i - 1));
    }

    for (i = 0; i < n; i++) {
        *(int *)cstl_vector_at(&v, i) = rand() % n;
    }
    cstl_vector_sort(&v, int_cmp, NULL, CSTL_VECTOR_SORT_ALGORITHM_HEAP);
    for (i = 1; i < n; i++) {
        ck_assert_int_ge(*(int *)cstl_vector_at(&v, i),
                         *(int *)cstl_vector_at(&v, i - 1));
    }

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
        const int x = i;
        ck_assert_int_eq(cstl_vector_search(&v, &x, int_cmp, NULL), i);
    }

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

Suite * vector_suite(void)
{
    Suite * const s = suite_create("vector");

    TCase * tc;

    tc = tcase_create("vector");
    tcase_add_test(tc, sort);
    tcase_add_test(tc, search);
    tcase_add_test(tc, reverse);
    suite_add_tcase(s, tc);

    return s;
}

#endif
