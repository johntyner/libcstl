/*!
 * @file
 */

#ifndef CSTL_RBTREE_H
#define CSTL_RBTREE_H

/*!
 * @defgroup rbtree Red-black tree
 * @ingroup noalloc bintree_related
 * @brief An self-balancing binary tree
 *
 * @internal
 * The red-black tree algorithm(s) contained herein come from
 * the book Introduction to Algorithms by Cormen, Leiserson, and
 * Rivest aka The Big Book of Algorithms.
 *
 * There are 4 rules applied to the tree:
 * 1. Every node is red or black
 * 2. NULL children are considered leaves and are always black
 * 3. If a node is red, both children must be black
 * 4. Every path from a node to a leaf has the same number of blacks
 * @endinternal
 */
/*!
 * @addtogroup rbtree
 * @{
 */

#include "bintree.h"

/*! @private */
typedef enum {
    RBTREE_COLOR_R,
    RBTREE_COLOR_B,
} rbtree_color_t;

/*!
 * @brief Node to anchor an element within a red-black tree
 *
 * Users of the red-black tree object declare this object within another
 * object as follows:
 * @code{.c}
 * struct object {
 *     ...
 *     struct rbtree_node tree_node;
 *     ...
 * };
 * @endcode
 *
 * When calling rbtree_init(), the caller passes the offset of @p tree_node
 * within their object as the @p off parameter of that function, e.g.
 * @code{.c}
 * offsetof(struct object, tree_node)
 * @endcode
 */
struct rbtree_node {
    /*! @privatesection */
    rbtree_color_t c;
    struct bintree_node n;
};

/*!
 * @brief Red-black tree object
 *
 * Callers declare or allocate an object of this type to instantiate
 * a red-black tree. Users are encouraged to declare (and initialize) this
 * object with the DECLARE_RBTREE() macro. Any other declaration or
 * allocation must be initialized via rbtree_init().
 */
struct rbtree {
    /*! @privatesection */
    struct bintree t;

    size_t off;
};

/*!
 * @brief Constant initialization of a rbtree object
 *
 * @param TYPE The type of object that the tree will hold
 * @param MEMB The name of the @p rbtree_node member within @p TYPE.
 * @param CMP A pointer to a function of type @p cstl_compare_func_t that
 *            will be used to compare elements in the tree
 * @param PRIV A pointer to a private data structure that will be passed
 *             to calls to the @p CMP function
 *
 * @see rbtree_node for a description of the relationship between
 *                  @p TYPE and @p MEMB
 */
#define RBTREE_INITIALIZER(TYPE, MEMB, CMP, PRIV)               \
    {                                                           \
        .t = BINTREE_INITIALIZER(TYPE, MEMB.n, CMP, PRIV),      \
        .off = offsetof(struct rbtree_node, n),                 \
    }
/*!
 * @brief (Statically) declare and initialize a red-black tree
 *
 * @param NAME The name of the variable being declared
 * @param TYPE The type of object that the tree will hold
 * @param MEMB The name of the @p rbtree_node member within @p TYPE.
 * @param CMP A pointer to a function of type @p cstl_compare_func_t that
 *            will be used to compare elements in the tree
 * @param PRIV A pointer to a private data structure that will be passed
 *             to calls to the @p CMP function
 *
 * @see rbtree_node for a description of the relationship between
 *                  @p TYPE and @p MEMB
 */
#define DECLARE_RBTREE(NAME, TYPE, MEMB, CMP, PRIV)                     \
    struct rbtree NAME = RBTREE_INITIALIZER(TYPE, MEMB, CMP, PRIV)

/*!
 * @brief Initialize a red-black tree object
 *
 * @param[in,out] t A pointer to the object to be initialized
 * @param[in] cmp A function that can compare objects in the tree
 * @param[in] priv A pointer to private data that will be
 *                 passed to the @p cmp function
 * @param[in] off The offset of the @p rbtree_node object within the
 *                object(s) that will be stored in the tree
 */
static inline void rbtree_init(struct rbtree * const t,
                               cstl_compare_func_t * const cmp,
                               void * const priv,
                               const size_t off)
{
    bintree_init(&t->t, cmp, priv, off + offsetof(struct rbtree_node, n));
    t->off = off;
}

/*!
 * @brief Get the number of objects in the tree
 *
 * @param[in] t A pointer to the red-black tree
 *
 * @return The number of objects in the tree
 */
static inline size_t rbtree_size(const struct rbtree * const t)
{
    return bintree_size(&t->t);
}

/*!
 * @brief Insert a new object into the tree
 *
 * @param[in] t A pointer to the red-black tree
 * @param[in] e A pointer to the object to be inserted
 *
 * The inserted object does not need to compare as unequal to any/all
 * other objects already in the tree. If the object is equal to one or
 * more objects already in the tree, it is up to the caller to distinguish
 * between them during a find or erase operation.
 *
 * The inserted object must not be modified (in such a way as to affect
 * its comparison with other objects in the tree) as that modification
 * can cause the assumptions about the ordering of elements within the
 * tree to become invalid and lead to undefined behavior.
 */
void rbtree_insert(struct rbtree * t, void * e);

/*!
 * @brief Find an element within a tree
 *
 * @param[in] t A pointer to the red-black tree
 * @param[in] e A pointer to an object to compare to those in the tree
 *
 * The tree will be searched for an element that compares as equal
 * to the @p e parameter as defined by the @p cmp function provided
 * when the tree was initialized.
 *
 * @return A pointer to the (first) object in the tree that matches
 * @retval NULL No matching object was found
 */
static inline const void * rbtree_find(
    const struct rbtree * const t, const void * const e)
{
    return bintree_find(&t->t, e);
}

/*!
 * @brief Remove an element from the tree
 *
 * @param[in] t A pointer to the red-black tree
 * @param[in] e A pointer to an object to compare to those in the tree
 *
 * The tree will be searched for an element that compares as equal
 * to the @p e parameter as defined by the @p cmp function provided
 * when the tree was initialized. The first element found that matches
 * will be removed from the tree, and a pointer to that element will
 * be returned.
 *
 * @return A pointer to the removed element
 * @retval NULL No element was found/removed
 */
void * rbtree_erase(struct rbtree * t, const void * e);

/*!
 * @brief Remove all elements from the tree
 *
 * @param[in] t A pointer to the red-black tree
 * @param[in] clr A pointer to a function to be called for each
 *                element in the tree
 *
 * All elements are removed from the tree and the @p clr function is
 * called for each element that was in the tree. The order in which
 * the elements are removed and @p clr is called is not specified, but
 * the callee may take ownership of an element at the time that @p clr
 * is called for that element and not before.
 *
 * Upon return from this function, the tree contains no elements, and
 * is as it was immediately after being initialized. No further operations
 * on the tree are necessary to make it ready to go out of scope or be
 * destroyed.
 */
static inline void rbtree_clear(struct rbtree * const t,
                                cstl_clear_func_t * const clr)
{
    bintree_clear(&t->t, clr);
}

/*!
 * @brief Swap the tree objects at the two given locations
 *
 * @param[in,out] a A pointer to a red-black tree
 * @param[in,out] b A pointer to a(nother) red-black tree
 *
 * The trees at the given locations will be swapped such that upon return,
 * @p a will contain the tree previously pointed to by @p b and vice versa.
 */
static inline void rbtree_swap(struct rbtree * const a, struct rbtree * const b)
{
    size_t t;
    bintree_swap(&a->t, &b->t);
    cstl_swap(&a->off, &b->off, &t, sizeof(t));
}

/*!
 * @brief Visit each element in a tree, calling a user-defined
 *        function for each visit
 *
 * @param[in] t A pointer to the red-black tree
 * @param[in] visit A pointer to a function to be called for each visit
 * @param[in] priv A pointer to a private data structure that will be passed
 *                 to each call to @p visit
 * @param[in] dir The direction in which to traverse the tree
 *
 * The function continues visiting elements in the tree so long as the
 * given @p visit function returns 0. If the @p visit function returns a
 * non-zero value, no more elements are visited, and the function returns
 * the non-zero value that halted visitations.
 *
 * @see bintree_visit_order_t
 */
static inline
int rbtree_foreach(const struct rbtree * const t,
                   bintree_const_visit_func_t * const visit, void * const priv,
                   const bintree_foreach_dir_t dir)
{
    return bintree_foreach(&t->t, visit, priv, dir);
}

/*!
 * @brief Determine the maximum and minimum heights of a tree
 *
 * @param[in] t A pointer to the red-black tree
 * @param[out] min The length of the shortest path from a root to a leaf
 * @param[out] max The length of the longest path from the root to a leaf
 */
static inline
void rbtree_height(const struct rbtree * const t,
                   size_t * const min, size_t * const max)
{
    bintree_height(&t->t, min, max);
}

/*!
 * @} rbtree
 */

#endif
