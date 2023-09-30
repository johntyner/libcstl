#include "vector.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void * __vector_at(struct vector * const v, const size_t i)
{
    return (void *)((uintptr_t)v->elem + i * v->size);
}

void * vector_at(struct vector * const v, const size_t i)
{
    if (i < v->count) {
        return __vector_at(v, i);
    }
    return NULL;
}

const void * vector_at_const(const struct vector * const v, const size_t i)
{
    return vector_at((struct vector *)v, i);
}

static void __velem_exch(void * const x, void * const y,
                         void * const t,
                         const size_t sz)
{
#define VELEM_EXCH(TYPE, A, B)                  \
    do {                                        \
        const TYPE c = *(TYPE *)A;              \
        *(TYPE *)A = *(TYPE *)B;                \
        *(TYPE *)B = c;                         \
    } while (0)

    switch (sz) {
    case sizeof(uint8_t):  VELEM_EXCH(uint8_t, x, y);  break;
    case sizeof(uint16_t): VELEM_EXCH(uint16_t, x, y); break;
    case sizeof(uint32_t): VELEM_EXCH(uint32_t, x, y); break;
    case sizeof(uint64_t): VELEM_EXCH(uint64_t, x, y); break;
    default:
        memcpy(t, x, sz);
        memcpy(x, y, sz);
        memcpy(y, t, sz);
        break;
    }
}

static int __vector_force_capacity(struct vector * const v, const size_t sz)
{
    /*
     * the vector always (quietly) stores space for one extra
     * element at the end to use as scratch space for exchanging
     * elements during sort and reverse operations
     */
    void * const e = realloc(v->elem, (sz + 1) * v->size);

    if (e == NULL) {
        return -1;
    }

    v->elem = e;
    v->cap  = sz;

    return 0;
}

static int __vector_set_capacity(struct vector * const v, const size_t sz)
{
    if (sz > v->cap) {
        return __vector_force_capacity(v, sz);
    }
    return 0;
}

void vector_set_capacity(struct vector * const v, const size_t sz)
{
    __vector_set_capacity(v, sz);
}

void vector_shrink_to_fit(struct vector * const v)
{
    if (v->cap > v->count) {
        if (__vector_force_capacity(v, v->count) == 0) {
            v->count = v->cap;
        }
    }
}

int __vector_resize(struct vector * const v, const size_t sz)
{
    int res = 0;

    while (v->count > sz) {
        v->count--;
        if (v->dest != NULL) {
            v->dest(__vector_at(v, v->count));
        }
    }

    res = __vector_set_capacity(v, sz);
    if (res == 0) {
        while (v->count < sz) {
            if (v->cons != NULL) {
                v->cons(__vector_at(v, v->count));
            }
            v->count++;
        }
    }

    return res;
}

void vector_resize(struct vector * const v, const size_t sz)
{
    if (__vector_resize(v, sz) != 0) {
        abort();
    }
}

void vector_clear(struct vector * const v)
{
    vector_resize(v, 0);

    free(v->elem);
    v->elem = NULL;
    v->cap  = 0;
}

static size_t vector_qsort_p(struct vector * const v,
                             size_t i, size_t j,
                             int (* const cmp)(const void *, const void *),
                             void * const t)
{
    const void * x = __vector_at(v, i);

    for (i--, j++;;) {
        void * p, * r;

        do {
            i++;
            p = __vector_at(v, i);
        } while (cmp(x, p) > 0);

        do {
            j--;
            r = __vector_at(v, j);
        } while (cmp(x, r) < 0);

        if (i >= j) {
            break;
        }

        if (x == p) {
            x = r;
        } else if (x == r) {
            x = p;
        }

        __velem_exch(p, r, t, v->size);
    }

    return j;
}

static void vector_qsort(struct vector * const v,
                         const size_t p, const size_t r,
                         int (* const cmp)(const void *, const void *),
                         void * const tmp)
{
    if (p < r) {
        const size_t q = vector_qsort_p(v, p, r, cmp, tmp);
        vector_qsort(v, p, q, cmp, tmp);
        vector_qsort(v, q + 1, r, cmp, tmp);
    }
}

static void vector_hsort_b(struct vector * const v, const size_t sz,
                           const unsigned int i,
                           int (* const cmp)(const void *, const void *),
                           void * const tmp)
{
    const unsigned int l = 2 * i;
    const unsigned int r = l + 1;

    unsigned int n;

    n = i;
    if (l < sz && cmp(__vector_at(v, l), __vector_at(v, i)) > 0) {
        n = l;
    }
    if (r < sz && cmp(__vector_at(v, r), __vector_at(v, n)) > 0) {
        n = r;
    }

    if (n != i) {
        __velem_exch(__vector_at(v, i), __vector_at(v, n), tmp, v->size);
        vector_hsort_b(v, sz, n, cmp, tmp);
    }
}

static void vector_hsort(struct vector * const v,
                         int (* const cmp)(const void *, const void *),
                         void * const tmp)
{
    unsigned int i;

    for (i = v->count / 2; i > 0; i--) {
        vector_hsort_b(v, v->count, i - 1, cmp, tmp);
    }

    for (i = v->count - 1; i > 0; i--) {
        __velem_exch(__vector_at(v, 0), __vector_at(v, i), tmp, v->size);
        vector_hsort_b(v, i, 0, cmp, tmp);
    }
}

void vector_sort(struct vector * const v,
                 int (* const cmp)(const void *, const void *),
                 const vector_sort_algorithm_t algo)
{
    switch (algo) {
    case VECTOR_SORT_ALGORITHM_QUICK:
        vector_qsort(v, 0, v->count - 1, cmp, __vector_at(v, v->count));
        break;
    case VECTOR_SORT_ALGORITHM_HEAP:
        vector_hsort(v, cmp, __vector_at(v, v->count));
        break;
    }
}

ssize_t vector_search(const struct vector * const v,
                      const void * const e,
                      int (* const cmp)(const void *, const void *))
{
    unsigned int i, j;

    for (i = 0, j = v->count - 1; i <= j;) {
        const unsigned int n = (i + j) / 2;
        const int eq = cmp(e, __vector_at((struct vector *)v, n));

        if (eq == 0) {
            return n;
        } else if (eq < 0) {
            j = n - 1;
        } else {
            i = n + 1;
        }
    }

    return -1;
}

void vector_reverse(struct vector * const v)
{
    unsigned int i, j;

    for (i = 0, j = v->count - 1; i < j; i++, j--) {
        __velem_exch(__vector_at(v, i), __vector_at(v, j),
                     __vector_at(v, v->count),
                     v->size);
    }
}

#ifdef __cfg_test__
#include <check.h>

static int int_cmp(const void * const a, const void * const b)
{
    return *(int *)a - *(int *)b;
}

START_TEST(sort)
{
    static size_t n = 71;

    struct vector v;
    unsigned int i;

    VECTOR_INIT(&v, int, NULL, NULL);
    vector_resize(&v, n);

    for (i = 0; i < n; i++) {
        *(int *)vector_at(&v, i) = rand() % n;
    }
    vector_sort(&v, int_cmp, VECTOR_SORT_ALGORITHM_QUICK);
    for (i = 1; i < n; i++) {
        ck_assert_int_ge(*(int *)vector_at(&v, i),
                         *(int *)vector_at(&v, i - 1));
    }

    for (i = 0; i < n; i++) {
        *(int *)vector_at(&v, i) = rand() % n;
    }
    vector_sort(&v, int_cmp, VECTOR_SORT_ALGORITHM_HEAP);
    for (i = 1; i < n; i++) {
        ck_assert_int_ge(*(int *)vector_at(&v, i),
                         *(int *)vector_at(&v, i - 1));
    }

    vector_clear(&v);
}
END_TEST

START_TEST(search)
{
    static size_t n = 63;

    struct vector v;
    unsigned int i;

    VECTOR_INIT(&v, int, NULL, NULL);

    vector_resize(&v, n);
    for (i = 0; i < n; i++) {
        *(int *)vector_at(&v, i) = i;
    }

    for (i = 0; i < n; i++) {
        const int x = i;
        ck_assert_int_eq(vector_search(&v, &x, int_cmp), i);
    }

    vector_clear(&v);
}
END_TEST

START_TEST(reverse)
{
    static size_t n = 27;

    struct vector v;
    unsigned int i;

    VECTOR_INIT(&v, int, NULL, NULL);

    vector_resize(&v, n);
    for (i = 0; i < n; i++) {
        *(int *)vector_at(&v, i) = i;
    }

    vector_reverse(&v);
    for (i = 1; i < n; i++) {
        ck_assert_int_le(*(int *)vector_at(&v, i),
                         *(int *)vector_at(&v, i - 1));
    }

    vector_clear(&v);
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
