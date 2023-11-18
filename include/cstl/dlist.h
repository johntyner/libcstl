/*!
 * @file
 */

#ifndef CSTL_DLIST_H
#define CSTL_DLIST_H

/*!
 * @defgroup lists Linked lists
 * @ingroup lowlevel
 * @brief Collection of linked lists
 */

/*!
 * @defgroup dlist Doubly-linked list
 * @ingroup lists
 * @brief A linked list allowing traversal in both directions
 */
/*!
 * @addtogroup dlist
 * @{
 */

#include "cstl/common.h"

/*!
 * @brief Node to anchor an element within a list
 *
 * Users of the list object declare this object within another
 * object as follows:
 * @code{.c}
 * struct object {
 *     ...
 *     struct cstl_dlist_node node;
 *     ...
 * };
 * @endcode
 *
 * When calling cstl_dlist_init(), the caller passes the offset of @p node
 * within their object as the @p off parameter of that function, e.g.
 * @code{.c}
 * offsetof(struct object, node)
 * @endcode
 */
struct cstl_dlist_node
{
    /*! @privatesection */
    struct cstl_dlist_node * p, * n;
};

/*!
 * @brief List object
 *
 * Callers declare or allocate an object of this type to instantiate
 * a list. Users are encouraged to declare (and initialize) this
 * object with the DECLARE_CSTL_DLIST() macro. Any other declaration or
 * allocation must be initialized via cstl_dlist_init().
 */
struct cstl_dlist
{
    /*! @privatesection */
    struct cstl_dlist_node h;
    size_t size;

    size_t off;
};

/*!
 * @brief Constant initialization of a list object
 *
 * @param NAME The name of the variable being initialized
 * @param TYPE The type of object that the list will hold
 * @param MEMB The name of the @p cstl_dlist_node member within @p TYPE.
 *
 * @see cstl_dlist_node for a description of the relationship between
 *                     @p TYPE and @p MEMB
 */
#define CSTL_DLIST_INITIALIZER(NAME, TYPE, MEMB)        \
    {                                                   \
        .h = {                                          \
            .p = &NAME.h,                               \
            .n = &NAME.h,                               \
        },                                              \
        .size = 0,                                      \
        .off = offsetof(TYPE, MEMB),                    \
    }
/*!
 * @brief (Statically) declare and initialize a list
 *
 * @param NAME The name of the variable being declared
 * @param TYPE The type of object that the list will hold
 * @param MEMB The name of the @p cstl_dlist_node member within @p TYPE.
 *
 * @see cstl_dlist_node for a description of the relationship between
 *                     @p TYPE and @p MEMB
 */
#define DECLARE_CSTL_DLIST(NAME, TYPE, MEMB)            \
    struct cstl_dlist NAME =                            \
        CSTL_DLIST_INITIALIZER(NAME, TYPE, MEMB)

/*!
 * @brief Initialize a list object
 *
 * @param[in,out] l A pointer to the object to be initialized
 * @param[in] off The offset of the @p cstl_dlist_node object within the
 *                object(s) that will be stored in the list
 */
static inline void cstl_dlist_init(
    struct cstl_dlist * const l, const size_t off)
{
    l->h.p  = &l->h;
    l->h.n  = &l->h;

    l->size = 0;

    l->off = off;
}

/*!
 * @brief Get the number of objects in the list
 *
 * @param[in] l A pointer to the list
 *
 * @return The number of objects in the list
 */
static inline size_t cstl_dlist_size(const struct cstl_dlist * const l)
{
    return l->size;
}

/*!
 * @brief Insert a new object into the list
 *
 * @param[in] l A pointer to the list into which to insert the object
 * @param[in] before A pointer to an object already in the list
 * @param[in] obj A pointer to an object to insert into the list
 *
 * The new object will be inserted after the object pointed to by @p before
 */
void cstl_dlist_insert(struct cstl_dlist * l, void * before, void * obj);

/*!
 * @brief Remove an object from the list
 *
 * @param[in] l The list in which the object currently resides
 * @param[in] obj A pointer to the object to be removed
 */
void cstl_dlist_erase(struct cstl_dlist * l, void * obj);

/*!
 * @brief Get a pointer to the first object in the list
 *
 * @param[in] l A pointer to a list
 *
 * @return A pointer to the first object in the list
 * @retval NULL The list is empty
 */
void * cstl_dlist_front(struct cstl_dlist * l);
/*!
 * @brief Get a pointer to the last object in the list
 *
 * @param[in] l A pointer to a list
 *
 * @return A pointer to the last object in the list
 * @retval NULL The list is empty
 */
void * cstl_dlist_back(struct cstl_dlist * l);

/*!
 * @brief Insert a new object at the front of the list
 *
 * @param[in] l A pointer to a list
 * @param[in] obj A pointer to the object to be inserted
 */
void cstl_dlist_push_front(struct cstl_dlist * l, void * obj);
/*!
 * @brief Insert a new object at the back of the list
 *
 * @param[in] l A pointer to a list
 * @param[in] obj A pointer to the object to be inserted
 */
void cstl_dlist_push_back(struct cstl_dlist * l, void * obj);

/*!
 * @brief Remove the first item in the list and return it
 *
 * @param[in] l A pointer to a list
 *
 * @return A pointer to the removed object
 * @retval NULL The list was empty and no object was removed
 */
void * cstl_dlist_pop_front(struct cstl_dlist * l);
/*!
 * @brief Remove the last item in the list and return it
 *
 * @param[in] l A pointer to a list
 *
 * @return A pointer to the removed object
 * @retval NULL The list was empty and no object was removed
 */
void * cstl_dlist_pop_back(struct cstl_dlist * l);

/*!
 * @brief Reverse the order of items in the list
 *
 * @param[in] l A pointer to the list
 */
void cstl_dlist_reverse(struct cstl_dlist * l);
/*!
 * @brief Sort the items in a list
 *
 * @param[in] l A pointer to the list
 * @param[in] cmp A pointer to a function to compare objects
 * @param[in] priv A pointer to be passed to each invocation
 *                 of the comparison function
 *
 * The items are sorted from least to greatest, according to the provided
 * comparison function.
 */
void cstl_dlist_sort(struct cstl_dlist * l,
                     cstl_compare_func_t * cmp, void * priv);

/*!
 * @brief Append one list to the end of another
 *
 * @param[in] list The list to which to append more objects
 * @param[in] more The list of objects to append
 *
 * Upon return @p list will have all of the objects from @p more
 * appended to its end.
 */
void cstl_dlist_concat(struct cstl_dlist * list, struct cstl_dlist * more);

/*!
 * @brief The direction in which to traverse the list
 *        during cstl_dlist_foreach()
 */
typedef enum
{
    /*! @brief Traverse the list from front to back */
    CSTL_DLIST_FOREACH_DIR_FWD,
    /*! @brief Traverse the list from back to front */
    CSTL_DLIST_FOREACH_DIR_REV,
} cstl_dlist_foreach_dir_t;

/*!
 * @brief Call a user-supplied function for each object in a list
 *
 * @param[in] l The list containing the objects to visit
 * @param[in] visit A pointer to the function to call for each object
 * @param[in] priv A pointer to be passed to each invocation of the function
 * @param[in] dir The direction in which to traverse the list
 *
 * The user-supplied function must return 0 in order to continue visiting
 * objects in the list. The visiting of objects will stop at the first
 * occurrence of a non-zero return from the visit function.
 *
 * @return The value returned from the user-supplied function from the
 *         last object visited or 0
 */
int cstl_dlist_foreach(struct cstl_dlist * l,
                       cstl_visit_func_t * visit, void * priv,
                       cstl_dlist_foreach_dir_t dir);

/*!
 * @brief Perform a linear search for an object
 *
 * @param[in] l The list to be searched
 * @param[in] obj A pointer to an object to search for in the list
 * @param[in] cmp A function to compare objects in the list
 * @param[in] priv A pointer to be passed to each invocation
 *            of the compare function
 * @param[in] dir The direction in which to traverse/search the list
 *
 * The function will traverse the list in the specified direction, comparing
 * the user-supplied object with objects in the list. When the comparison
 * function returns 0, the associated object is returned.
 *
 * @return A pointer to the sought object
 * @retval NULL The sought object was not found
 */
void * cstl_dlist_find(const struct cstl_dlist * l,
                       const void * obj, cstl_compare_func_t * cmp, void * priv,
                       cstl_dlist_foreach_dir_t dir);

/*!
 * @brief Remove objects from and reinitialize a list
 *
 * @param[in] l The list to be cleared
 * @param[in] clr The function to be called for each object in the list
 *
 * All objects are removed from the list and the @p clr function is
 * called for each object that was in the list. The order in which
 * the objects are removed and @p clr is called is not specified, but
 * the callee may take ownership of an object at the time that @p clr
 * is called for that object and not before.
 *
 * Upon return from this function, the list contains no objects, and it
 * is as it was immediately after being initialized. No further operations
 * on the list are necessary to make it ready to go out of scope or be
 * destroyed.
 */
void cstl_dlist_clear(struct cstl_dlist * l, cstl_xtor_func_t * clr);

/*!
 * @brief Swap the list objects at the two given locations
 *
 * @param[in,out] a A pointer to a list
 * @param[in,out] b A pointer to a(nother) list
 *
 * The lists at the given locations will be swapped such that upon return,
 * @p a will contain the list previously pointed to by @p b and vice versa.
 */
void cstl_dlist_swap(struct cstl_dlist * a, struct cstl_dlist * b);

/*!
 * @}
 */

#endif
