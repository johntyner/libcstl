/*!
 * @file
 */

#include "cstl/array.h"

static void * __cstl_raw_array_at(const void * const arr,
                                  const size_t size, const size_t at)
{
    return (void *)((uintptr_t)arr + (at * size));
}

void cstl_raw_array_reverse(void * const arr,
                            const size_t count, const size_t size,
                            cstl_swap_func_t * const swap,
                            void * const t)
{
    if (count > 1) {
        int i, j;

        for (i = 0, j = count - 1; i < j; i++, j--) {
            swap(__cstl_raw_array_at(arr, size, i),
                 __cstl_raw_array_at(arr, size, j),
                 t,
                 size);
        }
    }
}

ssize_t cstl_raw_array_search(const void * const arr,
                              const size_t count, const size_t size,
                              const void * const ex,
                              cstl_compare_func_t * const cmp,
                              void * const priv)
{
    if (count > 0) {
        int i, j;

        for (i = 0, j = count - 1; i <= j;) {
            const int n = (i + j) / 2;
            const int eq = cmp(ex, __cstl_raw_array_at(arr, size, n), priv);

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

ssize_t cstl_raw_array_find(const void * const arr,
                            const size_t count, const size_t size,
                            const void * const ex,
                            cstl_compare_func_t * const cmp,
                            void * const priv)
{
    size_t i;

    for (i = 0; i < count; i++) {
        if (cmp(ex, __cstl_raw_array_at(arr, size, i), priv) == 0) {
            return i;
        }
    }

    return -1;
}

/*! @private */
static size_t cstl_raw_array_qsort_p(
    void * const arr, const size_t count, const size_t size,
    size_t i, size_t j, const size_t p,
    cstl_compare_func_t * const cmp, void * const priv,
    cstl_swap_func_t * const swap, void * const t)
{
    const void * x = __cstl_raw_array_at(arr, size, p);

    for (i--, j++;;) {
        void * a, * b;

        do {
            i++;
            a = __cstl_raw_array_at(arr, size, i);
        } while (cmp(x, a, priv) > 0);

        do {
            j--;
            b = __cstl_raw_array_at(arr, size, j);
        } while (cmp(x, b, priv) < 0);

        if (i >= j) {
            break;
        }

        if (x == a) {
            x = b;
        } else if (x == b) {
            x = a;
        }

        swap(a, b, t, size);
    }

    return j;
    (void)count;
}

void cstl_raw_array_qsort(
    void * const arr, const size_t count, const size_t size,
    const size_t f, const size_t l,
    cstl_compare_func_t * const cmp, void * const priv,
    cstl_swap_func_t * const swap, void * const tmp,
    const int r)
{
    if (f < l) {
        size_t p = f;

        if (r != 0) {
            p = f + (rand() % (l - f + 1));
        }

        const size_t m = cstl_raw_array_qsort_p(
            arr, count, size,
            f, l, p,
            cmp, priv,
            swap, tmp);

        cstl_raw_array_qsort(arr, count, size,
                             f, m,
                             cmp, priv,
                             swap, tmp,
                             r);
        cstl_raw_array_qsort(arr, count, size,
                             m + 1, l,
                             cmp, priv,
                             swap, tmp,
                             r);
    }
}

/*! @private */
static void cstl_raw_array_hsort_b(
    void * const arr, const size_t count, const size_t size,
    const unsigned int i,
    cstl_compare_func_t * const cmp, void * const priv,
    cstl_swap_func_t * const swap, void * const tmp)
{
    const unsigned int l = 2 * i;
    const unsigned int r = l + 1;

    unsigned int n;

    n = i;
    if (l < count
        && cmp(__cstl_raw_array_at(arr, size, l),
               __cstl_raw_array_at(arr, size, i),
               priv) > 0) {
        n = l;
    }
    if (r < count
        && cmp(__cstl_raw_array_at(arr, size, r),
               __cstl_raw_array_at(arr, size, n),
               priv) > 0) {
        n = r;
    }

    if (n != i) {
        swap(__cstl_raw_array_at(arr, size, i),
             __cstl_raw_array_at(arr, size, n),
             tmp,
             size);
        cstl_raw_array_hsort_b(
            arr, count, size,
            n,
            cmp, priv,
            swap, tmp);
    }
}

void cstl_raw_array_hsort(
    void * const arr, const size_t count, const size_t size,
    cstl_compare_func_t * const cmp, void * const priv,
    cstl_swap_func_t * const swap, void * const tmp)
{
    unsigned int i;

    for (i = count / 2; i > 0; i--) {
        cstl_raw_array_hsort_b(arr, count, size,
                               i - 1,
                               cmp, priv,
                               swap, tmp);
    }

    for (i = count - 1; i > 0; i--) {
        swap(__cstl_raw_array_at(arr, size, 0),
             __cstl_raw_array_at(arr, size, i),
             tmp,
             size);
        cstl_raw_array_hsort_b(arr, i, size,
                               0,
                               cmp, priv,
                               swap, tmp);
    }
}

/*! @private */
struct cstl_raw_array
{
    /*! @privatesection */

    /*
     * @base consists of @nm elements,
     * each of @sz bytes
     */
    size_t sz, nm;
    void * buf;
};

void cstl_array_alloc(cstl_array_t * const a,
                      const size_t nm, const size_t sz)
{
    struct cstl_raw_array * ra;

    cstl_shared_ptr_reset(&a->ptr);
    cstl_shared_ptr_alloc(&a->ptr, sizeof(*ra) + nm * sz, NULL);

    ra = cstl_shared_ptr_get(&a->ptr);
    if (ra != NULL) {
        ra->sz = sz;
        ra->nm = nm;

        ra->buf = ra + 1;

        a->len = nm;
    }
}

void cstl_array_set(cstl_array_t * const a,
                    void * const buf, const size_t nm, const size_t sz)
{
    struct cstl_raw_array * ra;

    cstl_array_alloc(a, 0, sz);
    ra = cstl_shared_ptr_get(&a->ptr);
    if (ra != NULL) {
        ra->nm = nm;
        ra->buf = buf;
        a->len = nm;
    }
}

void cstl_array_release(cstl_array_t * const a, void ** const buf)
{
    const struct cstl_raw_array * const ra =
        cstl_shared_ptr_get_const(&a->ptr);
    void * b = NULL;

    if (ra != NULL
        && ra->buf != ra + 1
        && cstl_shared_ptr_unique(&a->ptr)) {
        b = ra->buf;
        cstl_array_reset(a);
    }

    if (buf != NULL) {
        *buf = b;
    }
}

const void * cstl_array_data_const(const cstl_array_t * const a)
{
    const struct cstl_raw_array * const ra =
        cstl_shared_ptr_get_const(&a->ptr);
    if (ra != NULL) {
        return ra->buf;
    }
    return NULL;
}

const void * cstl_array_at_const(const cstl_array_t * a, size_t i)
{
    if (i >= a->len) {
        abort(); // GCOV_EXCL_LINE
    } else {
        const struct cstl_raw_array * const ra =
            cstl_shared_ptr_get_const(&a->ptr);

        return (void *)((uintptr_t)ra->buf + (a->off + i) * ra->sz);
    }
}

void cstl_array_slice(cstl_array_t * const a,
                      const size_t beg, const size_t end,
                      cstl_array_t * const s)
{
    const struct cstl_raw_array * const ra =
        cstl_shared_ptr_get_const(&a->ptr);

    if (ra == NULL
        || end < beg
        || a->off + end > ra->nm) {
        abort(); // GCOV_EXCL_LINE
    }

    s->off = a->off + beg;
    s->len = end - beg;
    if (a != s) {
        cstl_shared_ptr_share(&a->ptr, &s->ptr);
    }
}

void cstl_array_unslice(cstl_array_t * const s, cstl_array_t * const a)
{
    const struct cstl_raw_array * const ra =
        cstl_shared_ptr_get_const(&s->ptr);
    if (ra == NULL) {
        abort(); // GCOV_EXCL_LINE
    }
    a->off = 0;
    a->len = ra->nm;
    if (a != s) {
        cstl_shared_ptr_share(&s->ptr, &a->ptr);
    }
}

#ifdef __cfg_test__
// GCOV_EXCL_START
#include <check.h>
#include <signal.h>

START_TEST(create)
{
    DECLARE_CSTL_ARRAY(a);
    cstl_array_alloc(&a, 30, sizeof(int));
    cstl_array_reset(&a);
}
END_TEST

START_TEST(slice)
{
    DECLARE_CSTL_ARRAY(a);
    DECLARE_CSTL_ARRAY(s);

    cstl_array_alloc(&a, 30, sizeof(int));
    ck_assert_int_eq(cstl_array_size(&a), 30);

    cstl_array_slice(&a, 20, 30, &s);
    ck_assert_int_eq(cstl_array_size(&s), 10);

    ck_assert_ptr_eq(cstl_array_at(&a, 20), cstl_array_at(&s, 0));

    cstl_array_reset(&a);
    ck_assert_int_eq(cstl_array_size(&a), 0);
    cstl_array_unslice(&s, &a);
    ck_assert_int_eq(cstl_array_size(&a), 30);

    cstl_array_reset(&a);
    ck_assert_int_eq(cstl_array_size(&s), 10);
    cstl_array_reset(&s);
}
END_TEST

START_TEST(set)
{
    void * p;
    int _[32];
    DECLARE_CSTL_ARRAY(a);
    DECLARE_CSTL_ARRAY(s);

    ck_assert_ptr_null(cstl_array_data(&a));
    cstl_array_set(&a, _, sizeof(_) / sizeof(*_), sizeof(*_));
    ck_assert_int_eq(cstl_array_size(&a), 32);

    ck_assert_ptr_eq(cstl_array_data(&a), _);

    cstl_array_slice(&a, 10, 20, &s);
    ck_assert_int_eq(cstl_array_size(&s), 10);

    cstl_array_reset(&s);
    cstl_array_release(&a, &p);
    ck_assert_ptr_eq(p, _);
}
END_TEST

START_TEST(access_before)
{
    DECLARE_CSTL_ARRAY(a);

    cstl_array_alloc(&a, 30, sizeof(int));
    ck_assert_int_eq(cstl_array_size(&a), 30);

    cstl_array_at(&a, -1);
}
END_TEST

START_TEST(access_after)
{
    DECLARE_CSTL_ARRAY(a);

    cstl_array_alloc(&a, 30, sizeof(int));
    ck_assert_int_eq(cstl_array_size(&a), 30);

    cstl_array_at(&a, 30);
}
END_TEST

START_TEST(big_slice)
{
    DECLARE_CSTL_ARRAY(a);
    DECLARE_CSTL_ARRAY(s);

    cstl_array_alloc(&a, 30, sizeof(int));
    ck_assert_int_eq(cstl_array_size(&a), 30);

    cstl_array_slice(&a, 20, 31, &s);
}
END_TEST

START_TEST(invalid_slice)
{
    DECLARE_CSTL_ARRAY(a);
    DECLARE_CSTL_ARRAY(s);

    cstl_array_alloc(&a, 30, sizeof(int));
    ck_assert_int_eq(cstl_array_size(&a), 30);

    cstl_array_slice(&a, 20, 10, &s);
}
END_TEST

Suite * array_suite(void)
{
    Suite * const s = suite_create("array");

    TCase * tc;

    tc = tcase_create("array");
    tcase_add_test(tc, create);
    tcase_add_test(tc, slice);
    tcase_add_test(tc, set);

    suite_add_tcase(s, tc);

    tc = tcase_create("abort");
    tcase_set_tags(tc, "abort");
    tcase_add_test_raise_signal(tc, access_before, SIGABRT);
    tcase_add_test_raise_signal(tc, access_after, SIGABRT);
    tcase_add_test_raise_signal(tc, big_slice, SIGABRT);
    tcase_add_test_raise_signal(tc, invalid_slice, SIGABRT);

    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
