/*!
 * @file
 */

#ifndef CSTL_HEAP_H
#define CSTL_HEAP_H

/*!
 * @defgroup heap Binary heap
 * @ingroup bintrees
 * @brief A binary tree organized as a heap
 *
 * A heap is a binary tree with the highest valued object (as determined
 * by the associated comparison function) at the root. Every node in the
 * tree is less than or equal to its parent. The highest valued object
 * in the tree can be found in constant time, and adding and removing
 * objects in the tree can be done in O(log n) where n is the number of
 * elements in the heap
 */
/*!
 * @addtogroup heap
 * @{
 */

#include "cstl/bintree.h"

/*!
 * @brief Node to anchor an element within a heap
 *
 * Users of the heap object declare this object within another
 * object as follows:
 * @code{.c}
 * struct object {
 *     ...
 *     struct cstl_heap_node heap_node;
 *     ...
 * };
 * @endcode
 *
 * When calling cstl_heap_init(), the caller passes the offset of
 * @p cstl_heap_node within their object as the @p off parameter of
 * that function, e.g.
 * @code{.c}
 * offsetof(struct object, heap_node)
 * @endcode
 */
struct cstl_heap_node
{
    /*! @privatesection */
    struct cstl_bintree_node bn;
};

/*!
 * @brief Heap object
 *
 * Callers declare or allocate an object of this type to instantiate
 * a heap. Users are encouraged to declare (and initialize) this
 * object with the DECLARE_CSTL_HEAP() macro. Any other declaration or
 * allocation must be initialized via cstl_heap_init().
 */
struct cstl_heap
{
    /*!
     * @privatesection
     *
     * interestingly the heap code doesn't ever have to
     * access the heap_node. it goes straight through to
     * the cstl_bintree_node; therefore, an off member (as found
     * in the cstl_bintree and rbtree objects isn't necessary)
     */
    struct cstl_bintree bt;
};

/*!
 * @brief Constant initialization of a heap object
 *
 * @param TYPE The type of object that the heap will hold
 * @param MEMB The name of the @p heap_node member within @p TYPE.
 * @param CMP A pointer to a function of type @p cstl_compare_func_t that
 *            will be used to compare elements in the heap
 * @param PRIV A pointer to a private data structure that will be passed
 *             to calls to the @p CMP function
 *
 * @see cstl_heap_node for a description of the relationship between
 *                @p TYPE and @p MEMB
 */
#define CSTL_HEAP_INITIALIZER(TYPE, MEMB, CMP, PRIV)                    \
    {                                                                   \
        .bt = CSTL_BINTREE_INITIALIZER(TYPE, MEMB.bn, CMP, PRIV),       \
    }
/*!
 * @brief (Statically) declare and initialize a heap
 *
 * @param NAME The name of the variable being declared
 * @param TYPE The type of object that the heap will hold
 * @param MEMB The name of the @p cstl_heap_node member within @p TYPE.
 * @param CMP A pointer to a function of type @p cstl_compare_func_t that
 *            will be used to compare elements in the heap
 * @param PRIV A pointer to a private data structure that will be passed
 *             to calls to the @p CMP function
 *
 * @see cstl_heap_node for a description of the relationship between
 *                @p TYPE and @p MEMB
 */
#define DECLARE_CSTL_HEAP(NAME, TYPE, MEMB, CMP, PRIV)  \
    struct cstl_heap NAME =                             \
        CSTL_HEAP_INITIALIZER(TYPE, MEMB, CMP, PRIV)

/*!
 * @brief Initialize a heap object
 *
 * @param[in,out] h A pointer to the object to be initialized
 * @param[in] cmp A function that can compare objects in the heap
 * @param[in] priv A pointer to private data that will be
 *                 passed to the @p cmp function
 * @param[in] off The offset of the @p cstl_heap_node object within the
 *                object(s) that will be stored in the heap
 */
static inline void cstl_heap_init(struct cstl_heap * const h,
                                  cstl_compare_func_t * const cmp,
                                  void * const priv,
                                  const size_t off)
{
    cstl_bintree_init(
        &h->bt, cmp, priv, off + offsetof(struct cstl_heap_node, bn));
}

/*!
 * @brief Get the number of objects in the heap
 *
 * @param[in] h A pointer to the heap
 *
 * @return The number of objects in the heap
 */
static inline size_t cstl_heap_size(const struct cstl_heap * const h)
{
    return cstl_bintree_size(&h->bt);
}

/*!
 * @brief Insert a new object into the heap
 *
 * @param[in] h A pointer to the heap
 * @param[in] e A pointer to the object to be inserted
 *
 * After insertion, the inserted object must not be modified (in such a
 * way as to affect its comparison with other objects in the heap) as
 * that modification can cause the assumptions about the ordering of
 * elements within the heap to become invalid and lead to undefined behavior.
 */
void cstl_heap_push(struct cstl_heap * h, void * e);

/*!
 * @brief Get a pointer to the object at the top of the heap
 *
 * @param[in] h A pointer to the heap
 *
 * @return A pointer to the object at the top of the heap
 * @retval NULL The heap is empty
 */
const void * cstl_heap_get(const struct cstl_heap * h);

/*!
 * @brief Remove the highest valued element from the heap
 *
 * @param[in] h A pointer to the heap
 *
 * @return The highest valued element in the heap
 * @retval NULL The heap is empty
 */
void * cstl_heap_pop(struct cstl_heap * h);

/*!
 * @brief Remove all elements from the heap
 *
 * @param[in] h A pointer to the heap
 * @param[in] clr A pointer to a function to be called for each
 *                element in the tree
 *
 * All elements are removed from the heap and the @p clr function is
 * called for each element that was in the heap. The order in which
 * the elements are removed and @p clr is called is not specified, but
 * the callee may take ownership of an element at the time that @p clr
 * is called for that element and not before.
 *
 * Upon return from this function, the heap contains no elements, and
 * is as it was immediately after being initialized. No further operations
 * on the tree are necessary to make it ready to go out of scope or be
 * destroyed.
 */
static inline void cstl_heap_clear(struct cstl_heap * const h,
                                   cstl_xtor_func_t * const clr)
{
    cstl_bintree_clear(&h->bt, clr, NULL);
}

/*!
 * @brief Swap the heap objects at the two given locations
 *
 * @param[in,out] a A pointer to a heap
 * @param[in,out] b A pointer to a(nother) heap
 *
 * The heaps at the given locations will be swapped such that upon return,
 * @p a will contain the heap previously pointed to by @p b and vice versa.
 */
static inline void cstl_heap_swap(
    struct cstl_heap * const a, struct cstl_heap * const b)
{
    cstl_bintree_swap(&a->bt, &b->bt);
}

/*!
 * @}
 */

#endif
