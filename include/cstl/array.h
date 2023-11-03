/*!
 * @file
 */

#ifndef CSTL_ARRAY_H
#define CSTL_ARRAY_H

#include "cstl/memory.h"

#include <sys/types.h>

/*!
 * @defgroup array Array
 * @ingroup highlevel
 * @brief Dynamically-allocated, fixed size array
 */
/*!
 * @addtogroup array
 * @{
 */

typedef struct
{
    /*! @privatesection */
    cstl_shared_ptr_t ptr;
    /* measured in number of elements */
    size_t off, len;
} cstl_array_t;

#define CSTL_ARRAY_INITIALIZER(NAME)                    \
    {                                                   \
        .ptr = CSTL_SHARED_PTR_INITIALIZER(NAME.ptr),   \
        .off = 0,                                       \
        .len = 0,                                       \
    }
#define DECLARE_CSTL_ARRAY(NAME)                \
    cstl_array_t NAME =                         \
        CSTL_ARRAY_INITIALIZER(NAME)

static inline void cstl_array_init(cstl_array_t * const a)
{
    cstl_shared_ptr_init(&a->ptr);
    a->off = a->len = 0;
}

static inline size_t cstl_array_size(const cstl_array_t * const a)
{
    return a->len;
}

void cstl_array_alloc(cstl_array_t * a, size_t nm, size_t sz);

static inline void cstl_array_reset(cstl_array_t * const a)
{
    cstl_shared_ptr_reset(&a->ptr);
    a->off = a->len = 0;
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

void cstl_array_slice(
    cstl_array_t * a, size_t beg, size_t end, cstl_array_t * s);

void cstl_array_unslice(cstl_array_t * s, cstl_array_t * a);

/*!
 * @}
 */

#endif
