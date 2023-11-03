/*!
 * @file
 */

#ifndef CSTL_MAP_H
#define CSTL_MAP_H

/*!
 * @defgroup map Map
 * @brief A container of key/value pairs with unique keys
 */
/*!
 * @addtogroup map
 * @{
 */

#include "cstl/rbtree.h"

#include <stdbool.h>

/*!
 * @brief The map object
 *
 * The map may be declared on the stack or allocated. In either
 * case, it must be initialized via cstl_map_init(). The map
 * must be cleared with cstl_map_clear() to free any memory alloced
 * by the map itself.
 */
typedef struct
{
    /*! @privatesection */
    struct cstl_rbtree t;
    struct {
        /*! @privatesection */
        cstl_compare_func_t * f;
        void * p;
    } cmp;
} cstl_map_t;

/*!
 * @brief A pointer to an element within the map
 */
typedef struct
{
    /*! @brief Pointer to the key associated with the element */
    const void * key;
    /*! @brief Pointer to the value contained by the element */
    void * val;

    /*! @private */
    void * _;
} cstl_map_iterator_t;

/*!
 * @name Iterators
 * @{
 */

/*!
 * @brief Return an iterator that refers to the end of the map
 *
 * Iterators for functions like cstl_map_find() and cstl_map_erase() that
 * do not find a corresponding entry in the map will return an iterator
 * that will compare equal with this value.
 *
 * @param[in] map A pointer to the map associated with the iterator
 *
 * @return A pointer to an iterator referring to the end of the map
 */
const cstl_map_iterator_t * cstl_map_iterator_end(const cstl_map_t * map);
/*!
 * @brief Compare two iterators for equality
 *
 * @param[in] a A pointer to a map iterator
 * @param[in] b A pointer to a map iterator
 *
 * @retval true The iterators are equal
 * @retval false The iterators are not equal
 */
static inline bool cstl_map_iterator_eq(
    const cstl_map_iterator_t * const a, const cstl_map_iterator_t * const b)
{
    return a->_ == b->_;
}

/*!
 * @}
 */

/*!
 * @brief Initialize a map
 *
 * Initializing a previously initialized map that has not been cleared
 * may cause the loss/leaking of memory.
 *
 * @param[out] map The map to be initialized
 * @param[in] cmp A function to be used to compare keys
 * @param[in] priv A pointer to be passed to each invocation of @p cmp
 */
void cstl_map_init(cstl_map_t * map, cstl_compare_func_t * cmp, void * priv);

/*!
 * @brief Return the number of elements in the map
 *
 * @param[in] map A pointer to the map
 *
 * @return The number of elements in the map
 */
static inline size_t cstl_map_size(const cstl_map_t * const map)
{
    return cstl_rbtree_size(&map->t);
}

/*!
 * @brief Insert a key/value pair into the map
 *
 * @param[in] map The map into which to insert the pair
 * @param[in] key A pointer to the key
 * @param[in] val A pointer to the value
 * @param[out] i A pointer to an iterator in which to return a pointer to
 *               the new or existing element in the map. This parameter
 *               may be NULL
 *
 * @retval -1 The function failed to allocate memory
 * @retval 0 The function successfully inserted the pair, and @p i
 *           points to the inserted element
 * @retval 1 An element with the same key already exists in the map. The
 *           new pair was not inserted, @p i points to the existing element
 *           in the map
 */
int cstl_map_insert(cstl_map_t * map,
                    const void * key, void * val,
                    cstl_map_iterator_t * i);

/*!
 * @brief Find an element in the map with a matching key
 *
 * @param[in] map A pointer to the map to be searched
 * @param[in] key A pointer to the key that is sought
 * @param[out] i A pointer to an iterator in which to return a pointer to the
 *               sought element. This parameter may not be NULL
 *
 * The @p i parameter will be "end" if the element is not found
 */
void cstl_map_find(const cstl_map_t * map, const void * key,
                   cstl_map_iterator_t * i);

/*!
 * @brief Erase the element with the supplied key from the map
 *
 * @param[in] map A pointer to the map in which to search for the key
 * @param[in] key A pointer to the key to be removed
 * @param[out] i A pointer to an iterator in which to return a pointer to the
 *               sought element. This parameter may be NULL
 *
 * The @p i parameter will compare as equal with "end" upon return; however,
 * if an element with a matching key existed, the @p key and @p val members
 * will point to the key and value, respectively that we contained in the
 * element.
 *
 * @retval 0 The element was found and removed
 * @retval -1 No matching element was found
 */
int cstl_map_erase(cstl_map_t * map, const void * key,
                   cstl_map_iterator_t * i);

/*!
 * @brief Erase the element pointed to by the iterator
 *
 * @param[in] map The map into which the iterator points
 * @param[in] i A pointer to the iterator indicating which element to removed
 */
void cstl_map_erase_iterator(cstl_map_t * map, cstl_map_iterator_t * i);

/*!
 * @brief Remove all elements from the map
 *
 * @param[in] map A pointer to the map to be cleared
 * @param[in] clr A function to call for each element as it is removed
 * @param[in] priv A pointer to be passed to each invocation of @p clr
 *
 * The first argument to the @p clr function will be an iterator containing
 * pointers to the key and value associated with the removed element. It is
 * undefined whether the element is still in the tree at the time that the
 * @p clr function is called, and the callee must not do anything with the
 * iterator except retrieve the key and value pointers.
 */
void cstl_map_clear(cstl_map_t * map, cstl_xtor_func_t * clr, void * priv);

/*!
 * @}
 */

#endif
