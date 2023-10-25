/*!
 * @file
 */

#ifndef CSTL_VECTOR_H
#define CSTL_VECTOR_H

/*!
 * @defgroup vector Vector
 * @brief Variable-sized array
 */
/*!
 * @addtogroup vector
 * @{
 */

#include "common.h"

/*!
 * @brief Vector object
 *
 * Callers declare or allocate an object of this type to instantiate
 * a vector. Users are encouraged to declare (and initialize) this
 * object with the DECLARE_VECTOR() macro. Any other declaration or
 * allocation must be initialized via vector_init().
 */
struct vector
{
    /*! @privatesection */
    struct {
        /*! @privatesection */
        void * base;
        size_t size;
        int ext;
    } elem;
    size_t count, cap;
};

/*!
 * @brief Constant initialization of a vector object
 *
 * @param TYPE The type of object that the vector will hold
 */
#define VECTOR_INITIALIZER(TYPE)                \
    {                                           \
        .elem = {                               \
            .base = NULL,                       \
            .size = sizeof(TYPE),               \
            .ext = !(0 == 0),                   \
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
#define DECLARE_VECTOR(NAME, TYPE)                              \
    struct vector NAME = VECTOR_INITIALIZER(TYPE)

/*!
 * @brief Initialize a vector object
 *
 * @param[in,out] v A pointer to the vector object
 * @param[in] sz The size of the type of object that will be
 *               stored in the vector
 * @param[in] buf A pointer to an externally allocated buffer to be
 *                used as the underlying vector data buffer. This may
 *                be NULL to indicate the vector should be dynamically
 *                allocated
 * @param[in] cap The number of objects that can be stored in the
 *                externally allocated buffer. The vector capacity is
 *                reduced by one for internal use
 */
static inline void vector_init(struct vector * const v, const size_t sz,
                               void * const buf, const size_t cap)
{
    v->elem.base = buf;
    v->elem.size = sz;
    v->elem.ext  = (buf != NULL);

    v->count = 0;
    if (v->elem.ext) {
        /*
         * the vector always keeps an element in reserve
         * as scratch space for certain functions.
         */
        v->cap = cap - 1;
    } else {
        v->cap = 0;
    }
}

/*!
 * @brief Get the number of elements in the vector
 *
 * @param[in] v A pointer to the vector object
 *
 * @return The number of elements in the vector
 */
static inline size_t vector_size(const struct vector * const v)
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
static inline size_t vector_capacity(const struct vector * const v)
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
static inline void * vector_data(struct vector * const v)
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
void * vector_at(struct vector * v, size_t i);
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
const void * vector_at_const(const struct vector * v, size_t i);

/*!
 * @brief Request to increase the capacity of the vector
 *
 * @param[in] v A pointer to the vector object
 * @param[in] sz The number of elements the vector should be able to hold
 *
 * Requests to decrease the capacity are ignored. Requests to increase
 * the capacity that fail do so quietly
 */
void vector_reserve(struct vector * v, size_t sz);
/*!
 * @brief Request to decrease the capacity of the vector
 *
 * @param[in] v A pointer to the vector object
 *
 * The function attempts to decrease the capacity of the vector
 * to only that required to hold the current number of valid elements.
 * The function may fail and will do so without error.
 */
void vector_shrink_to_fit(struct vector * v);

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
void vector_resize(struct vector * v, size_t sz);

/*!
 * @brief Enumeration indicating the desired sort algorithm for vector_sort()
 */
typedef enum {
    /*! @brief Quicksort */
    VECTOR_SORT_ALGORITHM_QUICK,
    /*! @brief Randomized quicksort */
    VECTOR_SORT_ALGORITHM_QUICK_R,
    /*! @brief Heapsort */
    VECTOR_SORT_ALGORITHM_HEAP,

    /*! @brief Unspecified default algorithm */
    VECTOR_SORT_ALGORITHM_DEFAULT = VECTOR_SORT_ALGORITHM_QUICK_R,
} vector_sort_algorithm_t;

/*!
 * @brief Sort the elements in the vector
 *
 * @param[in] v A pointer to the vector
 * @param[in] cmp A pointer to a function to use to compare elements
 * @param[in] priv A pointer to be passed to each invocation
 *            of the comparison function
 * @param[in] algo The algorithm to use for the sort
 */
void vector_sort(struct vector * v,
                 cstl_compare_func_t * cmp, void * priv,
                 vector_sort_algorithm_t algo);

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
ssize_t vector_search(const struct vector * v,
                      const void * e, cstl_compare_func_t * cmp, void * priv);

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
ssize_t vector_find(const struct vector * v,
                    const void * e, cstl_compare_func_t * cmp, void * priv);

/*!
 * @brief Reverse the current order of the elements
 *
 * @param[in] v A pointer to the vector
 */
void vector_reverse(struct vector * v);

/*!
 * @brief Swap the vector objects at the two given locations
 *
 * @param[in,out] a A pointer to a vector
 * @param[in,out] b A pointer to a(nother) vector
 *
 * The vectores at the given locations will be swapped such that upon return,
 * @p a will contain the vector previously pointed to by @p b and vice versa.
 */
void vector_swap(struct vector * a, struct vector * b);

/*!
 * @brief Return a vector to its initialized state
 *
 * @param[in] v A pointer to a vector
 */
void vector_clear(struct vector * v);

/*!
 * @} vector
 */

#endif
