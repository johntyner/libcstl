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

static void * cstl_raw_array_at(const struct cstl_raw_array * const ra,
                                const size_t i)
{
    if (ra == NULL || i >= ra->nm) {
        abort();
    }
    return (void *)((uintptr_t)(ra + 1) + i * ra->sz);
}

size_t cstl_array_size(const cstl_array_t * const a)
{
    const struct cstl_raw_array * const ra =
        cstl_shared_ptr_get_const(&a->ptr);
    if (ra != NULL) {
        return ra->sz;
    }
    return 0;
}

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
    }
}

const void * cstl_array_data_const(const cstl_array_t * const a)
{
    return cstl_raw_array_at(cstl_shared_ptr_get_const(&a->ptr), 0);
}

const void * cstl_array_at_const(const cstl_array_t * a, size_t i)
{
    return cstl_raw_array_at(cstl_shared_ptr_get_const(&a->ptr), i);
}

void cstl_array_slice(cstl_array_t * const a,
                      const size_t beg, const size_t len,
                      cstl_slice_t * const s)
{
    const struct cstl_raw_array * const ra =
        cstl_shared_ptr_get_const(&a->ptr);

    if (ra == NULL || beg + len > ra->nm) {
        abort();
    }

    s->off = beg;
    s->len = len;
    cstl_shared_ptr_share(&a->ptr, &s->ptr);
}

const void * cstl_slice_data_const(const cstl_slice_t * const s)
{
    return cstl_raw_array_at(cstl_shared_ptr_get_const(&s->ptr), s->off);
}

const void * cstl_slice_at_const(const cstl_slice_t * const s, const size_t i)
{
    if (i > s->len) {
        abort();
    }
    return cstl_raw_array_at(cstl_shared_ptr_get_const(&s->ptr), s->off + i);
}

void cstl_slice_adjust(cstl_slice_t * const s,
                       const ssize_t beg, const size_t len)
{
    const struct cstl_raw_array * const ra =
        cstl_shared_ptr_get_const(&s->ptr);
    if (ra == NULL || s->off + beg + len > ra->nm) {
        abort();
    }
    s->off += beg;
    s->len  = len;
}

void cstl_slice_slice(const cstl_slice_t * const is,
                      const ssize_t beg, const size_t len,
                      cstl_slice_t * const os)
{
    const struct cstl_raw_array * const ra =
        cstl_shared_ptr_get_const(&is->ptr);
    if (ra == NULL || is->off + beg + len > ra->nm) {
        abort();
    }
    cstl_shared_ptr_share(&is->ptr, &os->ptr);
    os->off = is->off + beg;
    os->len = len;
}

#ifdef __cfg_test__
#include <check.h>

Suite * array_suite(void)
{
    Suite * const s = suite_create("array");

    TCase * tc;

    tc = tcase_create("array");

    suite_add_tcase(s, tc);

    return s;
}

#endif
