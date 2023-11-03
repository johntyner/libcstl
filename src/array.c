/*!
 * @file
 */

#include "cstl/array.h"

/*! @private */
struct cstl_raw_array
{
    /*! @privatesection */

    /*
     * @base consists of @numb elements,
     * each of @size bytes
     */
    size_t sz, nm;
    /* array follows */
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

        a->len = nm;
    }
}

const void * cstl_array_data_const(const cstl_array_t * const a)
{
    return cstl_array_at_const(a, 0);
}

const void * cstl_array_at_const(const cstl_array_t * a, size_t i)
{
    if (i > a->len) {
        abort();
    } else if (a->len == 0) {
        return NULL;
    } else {
        const struct cstl_raw_array * const ra =
            cstl_shared_ptr_get_const(&a->ptr);

        return (void *)((uintptr_t)(ra + 1) + (a->off + i) * ra->sz);
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
        abort();
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
        abort();
    }
    a->off = 0;
    a->len = ra->nm;
    if (a != s) {
        cstl_shared_ptr_share(&s->ptr, &a->ptr);
    }
}

#ifdef __cfg_test__
#include <check.h>

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

    cstl_array_slice(&a, 1, 21, &s);
    ck_assert_int_eq(cstl_array_size(&s), 20);

    ck_assert_ptr_eq(cstl_array_at(&a, 1), cstl_array_at(&s, 0));

    cstl_array_reset(&a);
    ck_assert_int_eq(cstl_array_size(&a), 0);
    cstl_array_unslice(&s, &a);
    ck_assert_int_eq(cstl_array_size(&a), 30);

    cstl_array_reset(&a);
    ck_assert_int_eq(cstl_array_size(&s), 20);
    cstl_array_reset(&s);
}
END_TEST

Suite * array_suite(void)
{
    Suite * const s = suite_create("array");

    TCase * tc;

    tc = tcase_create("array");
    tcase_add_test(tc, create);
    tcase_add_test(tc, slice);

    suite_add_tcase(s, tc);

    return s;
}

#endif
