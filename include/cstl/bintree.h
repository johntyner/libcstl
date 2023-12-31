/*!
 * @file
 */

#ifndef CSTL_BINTREE_H
#define CSTL_BINTREE_H

/*!
 * @defgroup bintrees Binary trees
 * @ingroup lowlevel
 * @brief Objects implemented as binary trees
 */

/*!
 * @defgroup bintree Binary tree
 * @ingroup bintrees
 * @brief An unbalanced binary tree
 */
/*!
 * @addtogroup bintree
 * @{
 */

#include "cstl/common.h"

#include <stddef.h>

/*!
 * @brief Node to anchor an element within a binary tree
 *
 * Users of the binary tree object declare this object within another
 * object as follows:
 * @code{.c}
 * struct object {
 *     ...
 *     struct cstl_bintree_node tree_node;
 *     ...
 * };
 * @endcode
 *
 * When calling cstl_bintree_init(), the caller passes the offset of
 * @p tree_node within their object as the @p off parameter of that
 * function, e.g.
 * @code{.c}
 * offsetof(struct object, tree_node)
 * @endcode
 */
struct cstl_bintree_node
{
    /*! @privatesection */
    struct cstl_bintree_node * p, * l, * r;
};

/*!
 * @brief Binary tree object
 *
 * Callers declare or allocate an object of this type to instantiate
 * a binary tree. Users are encouraged to declare (and initialize) this
 * object with the DECLARE_CSTL_BINTREE() macro. Any other declaration or
 * allocation must be initialized via cstl_bintree_init().
 */
struct cstl_bintree
{
    /*! @privatesection */
    struct cstl_bintree_node * root;
    size_t size;

    size_t off;
    struct
    {
        /*! @privatesection */
        cstl_compare_func_t * func;
        void * priv;
    } cmp;
};

/*!
 * @brief Constant initialization of a cstl_bintree object
 *
 * @param TYPE The type of object that the tree will hold
 * @param MEMB The name of the @p cstl_bintree_node member within @p TYPE.
 * @param CMP A pointer to a function of type @p cstl_compare_func_t that
 *            will be used to compare elements in the tree
 * @param PRIV A pointer to a private data structure that will be passed
 *             to calls to the @p CMP function
 *
 * @see cstl_bintree_node for a description of the relationship between
 *                        @p TYPE and @p MEMB
 */
#define CSTL_BINTREE_INITIALIZER(TYPE, MEMB, CMP, PRIV) \
    {                                                   \
        .root = NULL,                                   \
        .size = 0,                                      \
        .off = offsetof(TYPE, MEMB),                    \
        .cmp = {                                        \
            .func = CMP,                                \
            .priv = PRIV                                \
        }                                               \
    }
/*!
 * @brief (Statically) declare and initialize a binary tree
 *
 * @param NAME The name of the variable being declared
 * @param TYPE The type of object that the tree will hold
 * @param MEMB The name of the @p cstl_bintree_node member within @p TYPE.
 * @param CMP A pointer to a function of type @p cstl_compare_func_t that
 *            will be used to compare elements in the tree
 * @param PRIV A pointer to a private data structure that will be passed
 *             to calls to the @p CMP function
 *
 * @see cstl_bintree_node for a description of the relationship between
 *                        @p TYPE and @p MEMB
 */
#define DECLARE_CSTL_BINTREE(NAME, TYPE, MEMB, CMP, PRIV)       \
    struct cstl_bintree NAME =                                  \
        CSTL_BINTREE_INITIALIZER(TYPE, MEMB, CMP, PRIV)

/*!
 * @brief Initialize a binary tree object
 *
 * @param[in,out] bt A pointer to the object to be initialized
 * @param[in] cmp A function that can compare objects in the tree
 * @param[in] priv A pointer to private data that will be
 *                 passed to the @p cmp function
 * @param[in] off The offset of the @p cstl_bintree_node object within the
 *                object(s) that will be stored in the tree
 */
static inline void cstl_bintree_init(struct cstl_bintree * const bt,
                                     cstl_compare_func_t * const cmp,
                                     void * const priv,
                                     const size_t off)
{
    bt->root    = NULL;
    bt->size    = 0;

    bt->off     = off;

    bt->cmp.func = cmp;
    bt->cmp.priv = priv;
}

/*!
 * @brief Get the number of objects in the tree
 *
 * @param[in] bt A pointer to the binary tree
 *
 * @return The number of objects in the tree
 */
static inline size_t cstl_bintree_size(const struct cstl_bintree * const bt)
{
    return bt->size;
}

/*!
 * @brief Insert a new object into the tree
 *
 * @param[in] bt A pointer to the binary tree
 * @param[in] e A pointer to the object to be inserted
 * @param[in] p A pointer to the parent of the object to be inserted.
 *              This pointer may be NULL or can be found via
 *              cstl_bintree_find()
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
void cstl_bintree_insert(struct cstl_bintree * bt, void * e, void * p);

/*!
 * @brief Find an element within a tree
 *
 * @param[in] bt A pointer to the binary tree
 * @param[in] e A pointer to an object to compare to those in the tree
 * @param[out] p The location in which to return a pointer to the parent
 *               of the found element (or where it would be located). This
 *               pointer may be NULL
 *
 * The tree will be searched for an element that compares as equal
 * to the @p e parameter as defined by the @p cmp function provided
 * when the tree was initialized.
 *
 * @return A pointer to the (first) object in the tree that matches
 * @retval NULL No matching object was found
 */
const void * cstl_bintree_find(
    const struct cstl_bintree * bt, const void * e, const void ** p);

/*!
 * @brief Remove an element from the tree
 *
 * @param[in] bt A pointer to the binary tree
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
void * cstl_bintree_erase(struct cstl_bintree * bt, const void * e);

/*!
 * @brief Remove all elements from the tree
 *
 * @param[in] bt A pointer to the binary tree
 * @param[in] clr A pointer to a function to be called for each
 *                element in the tree
 * @param[in] priv A pointer to be passed to each invocation of @p clr
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
void cstl_bintree_clear(struct cstl_bintree * bt,
                        cstl_xtor_func_t * clr, void * priv);

/*!
 * @brief Swap the tree objects at the two given locations
 *
 * @param[in,out] a A pointer to a binary tree
 * @param[in,out] b A pointer to a(nother) binary tree
 *
 * The trees at the given locations will be swapped such that upon return,
 * @p a will contain the tree previously pointed to by @p b and vice versa.
 */
void cstl_bintree_swap(struct cstl_bintree * a, struct cstl_bintree * b);

/*!
 * @brief Enumeration indicating the order in which a tree
 *        element is being visited during @p cstl_bintree_foreach()
 */
typedef enum
{
    /*! @brief The first visit to an element that has at least one child */
    CSTL_BINTREE_VISIT_ORDER_PRE,
    /*!
     * @brief The second visit to an element, after its
     *        first child has/would have been visited
     */
    CSTL_BINTREE_VISIT_ORDER_MID,
    /*!
     * @brief The last visit to an element, after both
     *        children have/would have been visited
     */
    CSTL_BINTREE_VISIT_ORDER_POST,
    /*! @brief The only visit to an element that has no children */
    CSTL_BINTREE_VISIT_ORDER_LEAF,
} cstl_bintree_visit_order_t;

/*!
 * @brief Enumeration indicating the order in which elements
 *        in a tree are visited during @p cstl_bintree_foreach()
 *
 * @note The enumerations and their descriptions assume that the
 *       tree's associated @p cmp function compares elements in the
 *       "normal/expected" fashion
 */
typedef enum
{
    /*! @brief Each element in the tree is visited from left-to-right */
    CSTL_BINTREE_FOREACH_DIR_FWD,
    /*! @brief Each element in the tree is visited from right-to-left */
    CSTL_BINTREE_FOREACH_DIR_REV,
} cstl_bintree_foreach_dir_t;

/*!
 * @brief The type of @a visit function associated with cstl_bintree_foreach()
 *
 * The @p cstl_bintree_foreach() function requires that visited elements are
 * presented as @a const because modifying the element could cause the
 * previously determined (in)equality relationships between elements to
 * be modified and for future operations on the tree to result in
 * undefined behavior
 *
 * @param[in] e A pointer to the element being visited
 * @param[in] ord An enumeration indicating which visit
 *                to this element is being performed
 * @param[in] p A pointer to a private data object belonging to the callee
 *
 * @retval 0 Continue visiting elements in the tree
 * @retval Nonzero Stop visiting elements in the tree
 */
typedef int cstl_bintree_const_visit_func_t(
    const void * e, cstl_bintree_visit_order_t ord, void * p);

/*!
 * @brief Visit each element in a tree, calling a user-defined
 *        function for each visit
 *
 * @param[in] bt A pointer to the binary tree
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
 * @see cstl_bintree_visit_order_t
 */
int cstl_bintree_foreach(const struct cstl_bintree * bt,
                         cstl_bintree_const_visit_func_t * visit, void * priv,
                         cstl_bintree_foreach_dir_t dir);

/*!
 * @brief Determine the maximum and minimum heights of a tree
 *
 * @param[in] bt A pointer to the binary tree
 * @param[out] min The length of the shortest path from a root to a leaf
 * @param[out] max The length of the longest path from the root to a leaf
 */
void cstl_bintree_height(const struct cstl_bintree * bt,
                         size_t * min, size_t * max);

/*! @private */
int __cstl_bintree_cmp(const struct cstl_bintree *,
                       const struct cstl_bintree_node *,
                       const struct cstl_bintree_node *);

/*! @private */
const struct cstl_bintree_node * __cstl_bintree_erase(
    struct cstl_bintree *, struct cstl_bintree_node *);

/*! @private */
typedef struct cstl_bintree_node ** (__cstl_bintree_child_func_t)(
    struct cstl_bintree_node *);

/*! @private */
static inline
struct cstl_bintree_node ** __cstl_bintree_left(
    struct cstl_bintree_node * const n)
{
    return &n->l;
}

/*! @private */
static inline
struct cstl_bintree_node ** __cstl_bintree_right(
    struct cstl_bintree_node * const n)
{
    return &n->r;
}

/*! @private */
void __cstl_bintree_rotate(struct cstl_bintree *,
                           struct cstl_bintree_node *,
                           __cstl_bintree_child_func_t *,
                           __cstl_bintree_child_func_t *);

/*!
 * @}
 */

#endif
