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

/*!
 * @brief The array object
 *
 * The object manages an allocated or externally supplied array
 * of elements and refers to all or a slice thereof. The memory
 * is managed such that its lifetime is preserved so long as any
 * array object still refers to it.
 */
typedef struct
{
    /*! @privatesection */
    cstl_shared_ptr_t ptr;
    /* measured in number of elements */
    size_t off, len;
} cstl_array_t;

/*!
 * @brief Initialize an array object at the time of declaration
 *
 * @param[in] NAME The name of the array object being initialized
 */
#define CSTL_ARRAY_INITIALIZER(NAME)                    \
    {                                                   \
        .ptr = CSTL_SHARED_PTR_INITIALIZER(NAME.ptr),   \
        .off = 0,                                       \
        .len = 0,                                       \
    }
/*!
 * @brief Declare (and initialize) an array object at compile time
 *
 * @param[in] NAME The name of the array object being declared
 */
#define DECLARE_CSTL_ARRAY(NAME)                \
    cstl_array_t NAME =                         \
        CSTL_ARRAY_INITIALIZER(NAME)

/*!
 * @brief Initialize a previously declared/allocated array object
 *
 * @param[in] a A pointer to an uninitialized array object
 */
static inline void cstl_array_init(cstl_array_t * const a)
{
    cstl_shared_ptr_init(&a->ptr);
    a->off = a->len = 0;
}

/*!
 * @brief Get the number of elements in the array
 *
 * @param[in] a A pointer to an array object
 *
 * @return The number of elements in the array
 */
static inline size_t cstl_array_size(const cstl_array_t * const a)
{
    return a->len;
}

/*!
 * @name Danger Zone
 * @brief Functions requiring extra careful handling
 *
 * Callers can use cstl_array_set() to use an externally allocated array,
 * but the caller must cstl_array_release() that array. Otherwise, the
 * code will try to return the array to the heap when the last reference
 * to it goes away.
 *
 * @{
 */

/*!
 * @brief Manage an externally allocated array
 *
 * @param[in,out] a A pointer to an initialized array object
 * @param[in] buf A pointer to an array containing @p nm elements,
 *                each of @p sz bytes
 * @param[in] nm The number of elements in the underlying array
 * @param[in] sz The size of each element in the underlying array
 */
void cstl_array_set(cstl_array_t * const a, void * buf, size_t nm, size_t sz);

/*!
 * @brief Release an externally allocated array
 *
 * If the underlying array was not externally allocated, this function
 * has no effect. Furthermore, there must be no other references to the
 * underlying array from other cstl_array_t objects for this function to
 * succeed. The underlying array will not be released while other references
 * still exist.
 *
 * @param[in] a A pointer to an array object
 * @param[out] buf A pointer into which to return the externally allocated
 *                 array. The array is returned on success or set to NULL
 *                 on failure. This parameter may be NULL
 */
void cstl_array_release(cstl_array_t * const a, void ** buf);

/*!
 * @}
 */

/*!
 * @brief Allocate an array to be managed
 *
 * @param[in,out] a A pointer to an initialized array object
 * @param[in] nm The number of elements in the underlying array
 * @param[in] sz The size of each element in the underlying array
 *
 * A failed allocation will leave the array object in an initialized state
 */
void cstl_array_alloc(cstl_array_t * a, size_t nm, size_t sz);

/*!
 * @brief Drop a reference to memory managed by this object
 *
 * If this object is the last reference to the underlying memory, that
 * memory will be freed.
 *
 * @param[in,out] a A pointer to an array object
 */
static inline void cstl_array_reset(cstl_array_t * const a)
{
    cstl_shared_ptr_reset(&a->ptr);
    a->off = a->len = 0;
}

/*!
 * @brief Return a pointer to the underlying array
 *
 * @param[in] a A pointer to the array object
 *
 * @return A pointer to the underlying array
 * @retval NULL The object does not manage an array
 */
const void * cstl_array_data_const(const cstl_array_t * a);
/*! @copydoc cstl_array_data_const() */
static inline void * cstl_array_data(cstl_array_t * const a)
{
    return (void *)cstl_array_data_const(a);
}

/*!
 * @brief Return a pointer to an element in the array
 *
 * @param[in] a A pointer to an array object
 * @param[in] i The index of the sought element in the array
 *
 * The function will abort() if the index is out of bounds.
 *
 * @return A pointer to the sought element
 */
const void * cstl_array_at_const(const cstl_array_t * a, size_t i);
/*! @copydoc cstl_array_at_const() */
static inline void * cstl_array_at(cstl_array_t * const a, const size_t i)
{
    return (void *)cstl_array_at_const(a, i);
}

/*!
 * @brief Create an array object referring to a slice of another
 *
 * @param[in] a A pointer to an array object
 * @param[in] beg The index in @p a at which the slice should begin
 * @param[in] end The index in @p a at which the slice should end
 * @param[in,out] s A pointer to an array object. @p s and @p a may point
 *                  to the same object
 *
 * If end < beg or if the slice would exceed the bounds of the array,
 * the function abort()s. Otherwise the slice refers to the elements
 * indicated by [beg, end).
 */
void cstl_array_slice(
    cstl_array_t * a, size_t beg, size_t end, cstl_array_t * s);

/*!
 * @brief Create an array object referring to the entire underlying array
 *
 * @param[in] a A pointer to an array object
 * @param[in,out] s A pointer to an array object. @p s and @p a may
 *                  point to the same object
 *
 * The resulting array object will refer to the entirety of the underlying
 * array, as originally allocated.
 */
void cstl_array_unslice(cstl_array_t * s, cstl_array_t * a);

/*!
 * @name Raw Array Functions
 * @{
 */
void cstl_raw_array_reverse(
    void * arr, size_t count, size_t size,
    cstl_swap_func_t * swap, void * t);
ssize_t cstl_raw_array_search(
    const void * arr, size_t count, size_t size,
    const void * ex, cstl_compare_func_t * cmp, void * priv);
ssize_t cstl_raw_array_find(
    const void * arr, size_t count, size_t size,
    const void * ex, cstl_compare_func_t * cmp, void * priv);
void cstl_raw_array_qsort(
    void * arr, size_t count, size_t size,
    size_t f, size_t l,
    cstl_compare_func_t * cmp, void * priv,
    cstl_swap_func_t * swap, void * tmp,
    int r);
void cstl_raw_array_hsort(
    void * arr, size_t count, size_t size,
    cstl_compare_func_t * cmp, void * priv,
    cstl_swap_func_t * swap, void * tmp);
/*!
 * @}
 */

/*!
 * @}
 */

#endif
