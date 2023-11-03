/*!
 * @file
 */

#ifndef CSTL_ARRAY_H
#define CSTL_ARRAY_H

#include "cstl/memory.h"

#include <sys/types.h>

/*!
 * @defgroup array
 */
/*!
 * @defgroup slice
 */

/*!
 * @ingroup array
 */
typedef struct
{
    /*! @privatesection */
    cstl_shared_ptr_t ptr;
} cstl_array_t;

/*!
 * @ingroup slice
 */
typedef struct
{
    /*! @privatesection */

    /* measured in number of elements */
    size_t off, len;
    cstl_shared_ptr_t ptr;
} cstl_slice_t;

/*!
 * @addtogroup array
 * @{
 */

static inline void cstl_array_init(cstl_array_t * const a)
{
    cstl_shared_ptr_init(&a->ptr);
}

size_t cstl_array_size(const cstl_array_t * a);
void cstl_array_alloc(cstl_array_t * a, size_t nm, size_t size);

static inline void cstl_array_reset(cstl_array_t * const a)
{
    cstl_shared_ptr_reset(&a->ptr);
}

const void * cstl_array_data_const(const cstl_array_t * a);
static inline void * cstl_array_data(cstl_array_t * const a)
{
    return (void *)cstl_array_data_const(a);
}

const void * cstl_array_at_const(const cstl_array_t * a, size_t i);
static inline void * cstl_array_at(cstl_array_t * const a, const size_t i)
{
    return (void *)cstl_array_at_const(a, i);
}

void cstl_array_slice(cstl_array_t * a,
                      size_t beg, size_t len,
                      cstl_slice_t * s);

/*!
 * @}
 */

/*!
 * @addtogroup slice
 * @{
 */

static inline void cstl_slice_init(cstl_slice_t * const s)
{
    s->off = s->len = 0;
    cstl_shared_ptr_init(&s->ptr);
}

static inline size_t cstl_slice_size(const cstl_slice_t * const s)
{
    return s->len;
}

static inline void cstl_slice_reset(cstl_slice_t * const s)
{
    cstl_shared_ptr_reset(&s->ptr);
    s->off = s->len = 0;
}

const void * cstl_slice_data_const(const cstl_slice_t * s);
static inline void * cstl_slice_data(cstl_slice_t * const s)
{
    return (void *)cstl_slice_data_const(s);
}

const void * cstl_slice_at_const(const cstl_slice_t * s, size_t i);
static inline void * cstl_slice_at(cstl_slice_t * const s, const size_t i)
{
    return (void *)cstl_slice_at_const(s, i);
}

void cstl_slice_adjust(cstl_slice_t * s, ssize_t beg, size_t len);
void cstl_slice_slice(const cstl_slice_t * is,
                      ssize_t beg, size_t len,
                      cstl_slice_t * os);

/*!
 * @}
 */

#endif
