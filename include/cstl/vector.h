/*!
 * @file
 */

#ifndef CSTL_VECTOR_H
#define CSTL_VECTOR_H

/*!
 * @defgroup vector Vector
 * @ingroup highlevel
 * @brief Variable-sized array
 */
/*!
 * @addtogroup vector
 * @{
 */

#include "cstl/common.h"

#include <sys/types.h>

/*!
 * @brief Vector object
 *
 * Callers declare or allocate an object of this type to instantiate
 * a vector. Users are encouraged to declare (and initialize) this
 * object with the DECLARE_CSTL_VECTOR() macro. Any other declaration or
 * allocation must be initialized via cstl_vector_init().
 */
typedef struct cstl_vector
{
    /*! @privatesection */
    struct
    {
        /*! @privatesection */
        void * base;
        size_t size;

        struct
        {
            /*! @privatesection */
            cstl_xtor_func_t * cons, * dest;
            void * priv;
        } xtor;
    } elem;
    size_t count, cap;
} cstl_vector_t;

/*!
 * @brief Constant initialization of a vector object
 *
 * @param TYPE The type of object that the vector will hold
 */
#define CSTL_VECTOR_INITIALIZER(TYPE)           \
    {                                           \
        .elem = {                               \
            .base = NULL,                       \
            .size = sizeof(TYPE),               \
            .xtor = {                           \
                .cons = NULL,                   \
                .dest = NULL,                   \
                .priv = NULL,                   \
            },                                  \
        },                                      \
        .count = 0,                             \
        .cap = 0,                               \
    }
/*!
 * @brief (Statically) declare and initialize a vector
 *
 * @param NAME The name of the variable being declared
 * @param TYPE The type of object that the vector will hold
 */
#define DECLARE_CSTL_VECTOR(NAME, TYPE)                         \
    struct cstl_vector NAME = CSTL_VECTOR_INITIALIZER(TYPE)

/*!
 * @brief Initialize a vector object
 *
 * This function is used when construction and/or destruction of
 * objects in the vector, as they go in and out of scope, is
 * necessary. Any or all of the construction/destruction parameters
 * may be NULL.
 *
 * Construction and/or destruction takes place (only) during a resize
 * of the vector and only affects elements coming into or going out
 * of scope, where in-scope refers to elements at locations less than
 * the "count" of the number of elements in the vector. In other words,
 * elements that exist between the size of the vector and the capacity
 * of the vector are considered uninitialized, either because they have
 * not yet been "constructed" or because they were "destroyed".
 *
 * @param[in,out] v A pointer to the vector object
 * @param[in] sz The size of the type of object that will be
 *               stored in the vector
 * @param[in] cons A pointer to a function to call for each element as
 *                 it comes into scope
 * @param[in] dest A pointer to a function to call for each element as
 *                 it goes out of scope
 * @param[in] priv A pointer to be passed to each call to @p cons or @p dest
 */
static inline void cstl_vector_init_complex(
    struct cstl_vector * const v, const size_t sz,
    cstl_xtor_func_t * const cons, cstl_xtor_func_t * const dest,
    void * const priv)
{
    v->elem.base = NULL;
    v->elem.size = sz;

    v->elem.xtor.cons = cons;
    v->elem.xtor.dest = dest;
    v->elem.xtor.priv = priv;

    v->count = 0;
    v->cap = 0;
}

/*!
 * @brief Initialize a vector object
 *
 * @param[in,out] v A pointer to the vector object
 * @param[in] sz The size of the type of object that will be
 *               stored in the vector
 */
static inline void cstl_vector_init(
    struct cstl_vector * const v, const size_t sz)
{
    cstl_vector_init_complex(v, sz, NULL, NULL, NULL);
}

/*!
 * @brief Get the number of elements in the vector
 *
 * @param[in] v A pointer to the vector object
 *
 * @return The number of elements in the vector
 */
static inline size_t cstl_vector_size(const struct cstl_vector * const v)
{
    return v->count;
}

/*!
 * @brief Get the number of elements the vector can hold
 *
 * The capacity of the vector can be changed, but the current capacity
 * indicates the number of elements that the vector can hold without
 * a memory reallocation
 *
 * @param[in] v A pointer to the vector object
 *
 * @return The number of elements the vector can hold
 */
static inline size_t cstl_vector_capacity(const struct cstl_vector * const v)
{
    return v->cap;
}

/*!
 * @brief Get a pointer to the start of the vector data
 *
 * @param[in] v A pointer to the vector object
 *
 * @return A pointer to the start of the vector data
 */
static inline void * cstl_vector_data(struct cstl_vector * const v)
{
    return v->elem.base;
}

/*!
 * @brief Get a pointer to an element in the vector
 *
 * @param[in] v A pointer to the vector
 * @param[in] i The 0-based index of the desired element in the vector
 *
 * @return A pointer to the element indicated by the index
 *
 * @note The code will cause an abort if the index is outside the range
 *       of valid elements it the vector
 */
void * cstl_vector_at(struct cstl_vector * v, size_t i);
/*!
 * @brief Get a const pointer to an element from a const vector
 *
 * @param[in] v A pointer to the vector
 * @param[in] i The 0-based index of the desired element in the vector
 *
 * @return A pointer to the element indicated by the index
 *
 * @note The code will cause an abort if the index is outside the range
 *       of valid elements it the vector
 */
const void * cstl_vector_at_const(const struct cstl_vector * v, size_t i);

/*!
 * @brief Request to increase the capacity of the vector
 *
 * @param[in] v A pointer to the vector object
 * @param[in] sz The number of elements the vector should be able to hold
 *
 * Requests to decrease the capacity are ignored. Requests to increase
 * the capacity that fail do so quietly
 */
void cstl_vector_reserve(struct cstl_vector * v, size_t sz);
/*!
 * @brief Request to decrease the capacity of the vector
 *
 * @param[in] v A pointer to the vector object
 *
 * The function attempts to decrease the capacity of the vector
 * to only that required to hold the current number of valid elements.
 * The function may fail and will do so without error.
 */
void cstl_vector_shrink_to_fit(struct cstl_vector * v);

/*!
 * @brief Change the number of valid elements in the vector
 *
 * @param[in] v A pointer to the vector
 * @param[in] sz The number of elements desired in the vector
 *
 * The function attempts to set the number of valid elements to the
 * number indicated. If the number is less than or equal to the
 * current capacity of the vector, the function always succeeds. If
 * the number exceeds the capacity, and the function cannot increase
 * the capacity, the function will cause an abort.
 */
void cstl_vector_resize(struct cstl_vector * v, size_t sz);

/*!
 * @brief Sort the elements in the vector
 *
 * @param[in] v A pointer to the vector
 * @param[in] cmp A pointer to a function to use to compare elements
 * @param[in] priv A pointer to be passed to each invocation
 *            of the comparison function
 * @param[in] swap A function to be used to swap elements within the vector.
 * @param[in] algo The algorithm to use for the sort
 */
void __cstl_vector_sort(struct cstl_vector * v,
                        cstl_compare_func_t * cmp, void * priv,
                        cstl_swap_func_t * swap,
                        cstl_sort_algorithm_t algo);

/*!
 * @brief Sort the elements in the vector
 *
 * @param[in] v A pointer to the vector
 * @param[in] cmp A pointer to a function to use to compare elements
 * @param[in] priv A pointer to be passed to each invocation
 *            of the comparison function
 *
 * @note Elements within the vector will be rearranged via a "simple copy".
 */
static inline void cstl_vector_sort(
    struct cstl_vector * const v,
    cstl_compare_func_t * const cmp, void * const priv)
{
    __cstl_vector_sort(v, cmp, priv, cstl_swap, CSTL_SORT_ALGORITHM_DEFAULT);
}

/*!
 * @brief Perform a binary search of the vector
 *
 * @param[in] v A pointer to a vector
 * @param[in] e A pointer to an object having the same value as the sought
 *              object according to the provided comparison function
 * @param[in] cmp A pointer to a function use to compare objects
 * @param[in] priv A pointer to be passed to each invocation of the
 *                 comparison function
 *
 * The behavior is undefined if the vector is not sorted.
 *
 * @return The index of the sought element
 * @retval -1 if the sought value is not found
 */
ssize_t cstl_vector_search(const struct cstl_vector * v,
                           const void * e,
                           cstl_compare_func_t * cmp, void * priv);

/*!
 * @brief Perform a linear search of the vector
 *
 * @param[in] v A pointer to a vector
 * @param[in] e A pointer to an object having the same value as the sought
 *              object according to the provided comparison function
 * @param[in] cmp A pointer to a function use to compare objects
 * @param[in] priv A pointer to be passed to each invocation of the
 *                 comparison function
 *
 * @return The index of the sought element
 * @retval -1 if the sought value is not found
 */
ssize_t cstl_vector_find(const struct cstl_vector * v,
                         const void * e,
                         cstl_compare_func_t * cmp, void * priv);

/*!
 * @brief Reverse the current order of the elements
 *
 * @param[in] v A pointer to the vector
 * @param[in] swap A function to be used to swap elements within the vector.
 */
void __cstl_vector_reverse(struct cstl_vector * v, cstl_swap_func_t * swap);

/*!
 * @brief Reverse the current order of the elements
 *
 * @param[in] v A pointer to the vector
 *
 * @note Elements within the vector will be rearranged via a "simple copy".
 */
static inline void cstl_vector_reverse(struct cstl_vector * const v)
{
    __cstl_vector_reverse(v, cstl_swap);
}

/*!
 * @brief Swap the vector objects at the two given locations
 *
 * @param[in,out] a A pointer to a vector
 * @param[in,out] b A pointer to a(nother) vector
 *
 * The vectors at the given locations will be swapped such that upon return,
 * @p a will contain the vector previously pointed to by @p b and vice versa.
 */
void cstl_vector_swap(struct cstl_vector * a, struct cstl_vector * b);

/*!
 * @brief Return a vector to its initialized state
 *
 * @param[in] v A pointer to a vector
 */
void cstl_vector_clear(struct cstl_vector * v);

/*!
 * @}
 */

#endif
