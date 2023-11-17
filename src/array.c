/*!
 * @file
 */

#include "cstl/array.h"

/*! @private */
static inline void * __cstl_raw_array_at(const void * const arr,
                                         const size_t size, const size_t at)
{
    return (void *)((uintptr_t)arr + (at * size));
}

void cstl_raw_array_reverse(void * const arr,
                            const size_t count, const size_t size,
                            cstl_swap_func_t * const swap,
                            void * const t)
{
    int i, j;

    for (i = 0, j = count - 1; i < j; i++, j--) {
        swap(__cstl_raw_array_at(arr, size, i),
             __cstl_raw_array_at(arr, size, j),
             t,
             size);
    }
}

ssize_t cstl_raw_array_search(const void * const arr,
                              const size_t count, const size_t size,
                              const void * const ex,
                              cstl_compare_func_t * const cmp,
                              void * const priv)
{
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

/*!
 * @private
 *
 * given a value (at the location p), walk inward from the two ends
 * while the left end is smaller than the given value and the right end
 * is bigger than the given value. if, when both traversals have stopped,
 * the locations of the stopped are still in their respective halves of
 * the array, swap the values at the locations. (this moves the values
 * greater than the chosen "middle" to the lower half and vice versa.)
 * continue the process until the respective indexes cross
 */
static size_t cstl_raw_array_qsort_p(
    void * const arr, const size_t count, const size_t size,
    const void * p,
    cstl_compare_func_t * const cmp, void * const priv,
    cstl_swap_func_t * const swap, void * const t)
{
    size_t i, j;
    void * a, * b;

    i = 0; j = count - 1;
    a = b = NULL;

    do {
        if (a != b) {
            swap(a, b, t, size);
            /*
             * it's possible that the chosen value that we're
             * "pivoting" around gets swapped. if that occurs,
             * keep track of its location so that the correct
             * value continues to be used for comparisons.
             */
            if (p == a) {
                p = b;
            } else if (p == b) {
                p = a;
            }

            i++; j--;
        }

        /*
         * walk forward from the beginning until a value greater than
         * or equal to the pivot is found
         */
        while (cmp(a = __cstl_raw_array_at(arr, size, i), p, priv) < 0) {
            i++;
        }

        /*
         * walk backward from the end until a value less than or equal
         * to the pivot value is found
         */
        while (cmp(b = __cstl_raw_array_at(arr, size, j), p, priv) > 0) {
            j--;
        }
    } while (i < j);

    return j;
}

/*! @private */
static void cstl_raw_array_qsort(
    void * const arr, const size_t count, const size_t size,
    cstl_compare_func_t * const cmp, void * const priv,
    cstl_swap_func_t * const swap, void * const tmp,
    const cstl_sort_algorithm_t algo)
{
    if (count > 1) {
        size_t p;

        /*
         * the choice of the pivot location/value is the
         * subject of much debate. a bad pivot choice will
         * result in the worst case behavior of the algorithm.
         * this code implements a couple of common mitigation
         * strategies.
         */

        if (algo == CSTL_SORT_ALGORITHM_QUICK_R) {
            /*
             * choose the pivot randomly. there's no guarantee
             * that we won't encounter worst-case behavior, but
             * randomization combats someone intentionally trying
             * to slow performance by choosing a bad initial
             * ordering. that said, the rand() function isn't
             * beyond being broken/anticipated, and generating a
             * random number is not without cost
             */
            p = rand() % count;
        } else if (algo == CSTL_SORT_ALGORITHM_QUICK_M) {
            /*
             * the median-of-three scheme looks at the first, middle,
             * and last elements in the array. it sorts them, and then
             * uses the middle value/location as the pivot. note that,
             * in the case of a 2 or 3 element array, this operation
             * results in a completely sorted array. on average, this
             * version wins on speed because it avoids another
             * partitioning and recursion below.
             */
            void * const beg = __cstl_raw_array_at(arr, size, 0);
            void * const end = __cstl_raw_array_at(arr, size, count - 1);
            void * mid;

            p = (count - 1) / 2;
            mid = __cstl_raw_array_at(arr, size, p);

            /*
             * there are six possibilities for the ordering of elements.
             * one possibility is that they are already in order. the
             * remaining possibilities require either one or two swaps.
             * three of those require swapping the outer elements, and
             * after doing so, two of those convert to one of the
             * remaining two possibilities that only require one swap.
             */
            if (cmp(end, beg, priv) < 0) {
                swap(end, beg, tmp, size);
            }
            if (cmp(mid, beg, priv) < 0) {
                swap(mid, beg, tmp, size);
            } else if (cmp(end, mid, priv) < 0) {
                swap(end, mid, tmp, size);
            }
        } else {
            /* basic quicksort; just use the first element */
            p = 0;
        }

        if (algo != CSTL_SORT_ALGORITHM_QUICK_M || count > 3) {
            /*
             * if the array is not already sorted, partition
             * the array around the pivot value.
             */
            const size_t m = cstl_raw_array_qsort_p(
                arr, count, size,
                __cstl_raw_array_at(arr, size, p),
                cmp, priv,
                swap, tmp);

            /*
             * sort the arrays on either side of the partition.
             * note that these calls see their partition as the
             * entire array. they don't understand that they
             * might be sorting portions of a larger array
             */
            cstl_raw_array_qsort(
                arr, m + 1, size,
                cmp, priv,
                swap, tmp,
                algo);
            cstl_raw_array_qsort(
                __cstl_raw_array_at(arr, size, m + 1), count - m - 1, size,
                cmp, priv,
                swap, tmp,
                algo);
        }
    }
}

/*!
 * @private
 *
 * this function assumes that the array is a heap with the root node
 * at element 0 with each node's children located at 2n+1 and 2n+2.
 *
 * for the purpose of this function, descendants of n are assumed to
 * already be in the correct locations to form a heap with n as their
 * root. n may or may not be in the correct location with respect to
 * its descendants, and this function will push n down through its
 * descendants until the heap rooted at the original location is valid.
 */
static void cstl_raw_array_hsort_b(
    void * const arr, const size_t count, const size_t size,
    size_t n,
    cstl_compare_func_t * const cmp, void * const priv,
    cstl_swap_func_t * const swap, void * const tmp)
{
    size_t c;

    c = SIZE_MAX;
    do {
        size_t l, r;

        if (c < SIZE_MAX) {
            /*
             * this block swaps n with its child on every
             * iteration of the loop except the first
             */
            swap(__cstl_raw_array_at(arr, size, n),
                 __cstl_raw_array_at(arr, size, c),
                 tmp,
                 size);
            n = c;
        }

        l = 2 * n + 1;
        r = l + 1;

        /*
         * the goal here is to find the greatest of n and its
         * children. after the block below, c either points to
         * n or the greater child of n that is (also) greater
         * than n. if, at the end, c points to n, then n is
         * already in the correct position, and the job is done.
         * if not, then loop around, push n down, and try again.
         */

        c = n;
        if (l < count
            && cmp(__cstl_raw_array_at(arr, size, l),
                   __cstl_raw_array_at(arr, size, c),
                   priv) > 0) {
            c = l;
        }
        if (r < count
            && cmp(__cstl_raw_array_at(arr, size, r),
                   __cstl_raw_array_at(arr, size, c),
                   priv) > 0) {
            c = r;
        }
    } while (n != c);
}

/*! @private */
void cstl_raw_array_hsort(
    void * const arr, const size_t count, const size_t size,
    cstl_compare_func_t * const cmp, void * const priv,
    cstl_swap_func_t * const swap, void * const tmp)
{
    if (count > 1) {
        ssize_t i;

        /*
         * assume the array is organized as a binary tree rooted at 0
         * with child nodes at 2n+1 and 2n+2. to make a heap out it,
         * first assume that all leaves correctly form individual heaps
         * of one element each.
         *
         * the loop below skips all the leaf elements and starts with the
         * first element that has one or more children. the hsort_b()
         * function reorders that element with respect to its children such
         * that they form a heap. the loop then continues with each node,
         * moving toward the root. at each step, all descendants of the
         * current element form a heap with only the current element
         * (possibly) being out of place
         */
        for (i = count / 2 - 1; i >= 0; i--) {
            cstl_raw_array_hsort_b(
                arr, count, size, i, cmp, priv, swap, tmp);
        }

        /*
         * with the heap now formed, the greatest element is at the front
         * of the array. swap the front element with the last element. this
         * has the effect of moving the greatest item into its correct,
         * sorted position and invalidating the heap by placing a (likely)
         * incorrect item at the top. shorten the array by one, and then
         * fix the heap by pushing the new, incorrect root down to the
         * correct position. the new heap is formed with one less item,
         * at which point, the process repeats.
         */
        for (i = count - 1; i > 0; i--) {
            swap(arr, __cstl_raw_array_at(arr, size, i), tmp, size);
            cstl_raw_array_hsort_b(
                arr, i, size, 0, cmp, priv, swap, tmp);
        }
    }
}

void cstl_raw_array_sort(
    void * const arr, const size_t count, const size_t size,
    cstl_compare_func_t * const cmp, void * const priv,
    cstl_swap_func_t * const swap, void * const tmp,
    const cstl_sort_algorithm_t algo)
{
    switch (algo) {
    case CSTL_SORT_ALGORITHM_QUICK:
    case CSTL_SORT_ALGORITHM_QUICK_R:
    case CSTL_SORT_ALGORITHM_QUICK_M:
        cstl_raw_array_qsort(arr, count, size, cmp, priv, swap, tmp, algo);
        break;
    case CSTL_SORT_ALGORITHM_HEAP:
        cstl_raw_array_hsort(arr, count, size, cmp, priv, swap, tmp);
        break;
    default:
        cstl_raw_array_sort(
            arr, count, size, cmp, priv, swap, tmp,
            CSTL_SORT_ALGORITHM_DEFAULT);
        break;
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
        CSTL_ABORT();
    } else {
        const struct cstl_raw_array * const ra =
            cstl_shared_ptr_get_const(&a->ptr);

        return __cstl_raw_array_at(ra->buf, ra->sz, a->off + i);
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
        CSTL_ABORT();
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
        CSTL_ABORT();
    }
    a->off = 0;
    a->len = ra->nm;
    if (a != s) {
        cstl_shared_ptr_share(&s->ptr, &a->ptr);
    }
}

#ifdef __cstl_cfg_test__
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

    ck_assert_signal(SIGABRT, cstl_array_unslice(&s, &s));
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

    ck_assert_signal(SIGABRT, cstl_array_at(&a, -1));

    cstl_array_reset(&a);
}
END_TEST

START_TEST(access_after)
{
    DECLARE_CSTL_ARRAY(a);

    cstl_array_alloc(&a, 30, sizeof(int));
    ck_assert_int_eq(cstl_array_size(&a), 30);

    ck_assert_signal(SIGABRT, cstl_array_at(&a, 30));

    cstl_array_reset(&a);
}
END_TEST

START_TEST(big_slice)
{
    DECLARE_CSTL_ARRAY(a);
    DECLARE_CSTL_ARRAY(s);

    cstl_array_alloc(&a, 30, sizeof(int));
    ck_assert_int_eq(cstl_array_size(&a), 30);

    ck_assert_signal(SIGABRT, cstl_array_slice(&a, 20, 31, &s));

    cstl_array_reset(&a);
}
END_TEST

START_TEST(invalid_slice)
{
    DECLARE_CSTL_ARRAY(a);
    DECLARE_CSTL_ARRAY(s);

    cstl_array_alloc(&a, 30, sizeof(int));
    ck_assert_int_eq(cstl_array_size(&a), 30);

    ck_assert_signal(SIGABRT, cstl_array_slice(&a, 20, 10, &s));

    cstl_array_reset(&a);
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
    tcase_add_test(tc, access_before);
    tcase_add_test(tc, access_after);
    tcase_add_test(tc, big_slice);
    tcase_add_test(tc, invalid_slice);

    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
