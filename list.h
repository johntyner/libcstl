/*!
 * @file
 */

#ifndef CSTL_LIST_H
#define CSTL_LIST_H

/*!
 * @defgroup list Doubly-linked list
 * @brief A linked list allowing traversal in both directions
 */
/*!
 * @addtogroup list
 * @{
 */

#include "common.h"

/*!
 * @brief Node to anchor an element within a list
 *
 * Users of the list object declare this object within another
 * object as follows:
 * @code{.c}
 * struct object {
 *     ...
 *     struct list_node node;
 *     ...
 * };
 * @endcode
 *
 * When calling list_init(), the caller passes the offset of @p node
 * within their object as the @p off parameter of that function, e.g.
 * @code{.c}
 * offsetof(struct object, node)
 * @endcode
 */
struct list_node
{
    /*! @privatesection */
    struct list_node * p, * n;
};

/*!
 * @brief List object
 *
 * Callers declare or allocate an object of this type to instantiate
 * a list. Users are encouraged to declare (and initialize) this
 * object with the DECLARE_LIST() macro. Any other declaration or
 * allocation must be initialized via list_init().
 */
struct list
{
    /*! @privatesection */
    struct list_node h;
    size_t size;

    size_t off;
};

/*!
 * @brief Constant initialization of a list object
 *
 * @param NAME The name of the variable being initialized
 * @param TYPE The type of object that the list will hold
 * @param MEMB The name of the @p list_node member within @p TYPE.
 *
 * @see list_node for a description of the relationship between
 *                @p TYPE and @p MEMB
 */
#define LIST_INITIALIZER(NAME, TYPE, MEMB)      \
    {                                           \
        .h = {                                  \
            .p = &NAME.h,                       \
            .n = &NAME.h,                       \
        },                                      \
        .size = 0,                              \
        .off = offsetof(TYPE, MEMB),            \
    }
/*!
 * @brief (Statically) declare and initialize a list
 *
 * @param NAME The name of the variable being declared
 * @param TYPE The type of object that the list will hold
 * @param MEMB The name of the @p list_node member within @p TYPE.
 *
 * @see list_node for a description of the relationship between
 *                @p TYPE and @p MEMB
 */
#define DECLARE_LIST(NAME, TYPE, MEMB)                          \
    struct list NAME = LIST_INITIALIZER(NAME, TYPE, MEMB)

/*!
 * @brief Initialize a list object
 *
 * @param[in,out] l A pointer to the object to be initialized
 * @param[in] off The offset of the @p list_node object within the
 *                object(s) that will be stored in the list
 */
static inline void list_init(struct list * const l, const size_t off)
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
static inline size_t list_size(const struct list * const l)
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
void list_insert(struct list * l, void * before, void * obj);

/*!
 * @brief Remove an object from the list
 *
 * @param[in] l The list in which the object currently resides
 * @param[in] obj A pointer to the object to be removed
 */
void list_erase(struct list * l, void * obj);

/*!
 * @brief Get a pointer to the first object in the list
 *
 * @param[in] l A pointer to a list
 *
 * @return A pointer to the first object in the list
 * @retval NULL The list is empty
 */
void * list_front(struct list * l);
/*!
 * @brief Get a pointer to the last object in the list
 *
 * @param[in] l A pointer to a list
 *
 * @return A pointer to the last object in the list
 * @retval NULL The list is empty
 */
void * list_back(struct list * l);

/*!
 * @brief Insert a new object at the front of the list
 *
 * @param[in] l A pointer to a list
 * @param[in] obj A pointer to the object to be inserted
 */
void list_push_front(struct list * l, void * obj);
/*!
 * @brief Insert a new object at the back of the list
 *
 * @param[in] l A pointer to a list
 * @param[in] obj A pointer to the object to be inserted
 */
void list_push_back(struct list * l, void * obj);

/*!
 * @brief Remove the first item in the list and return it
 *
 * @param[in] l A pointer to a list
 *
 * @return A pointer to the removed object
 * @retval NULL The list was empty and no object was removed
 */
void * list_pop_front(struct list * l);
/*!
 * @brief Remove the last item in the list and return it
 *
 * @param[in] l A pointer to a list
 *
 * @return A pointer to the removed object
 * @retval NULL The list was empty and no object was removed
 */
void * list_pop_back(struct list * l);

/*!
 * @brief Reverse the order of items in the list
 *
 * @param[in] l A pointer to the list
 */
void list_reverse(struct list * l);
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
void list_sort(struct list *, cstl_compare_func_t *, void *);

/*!
 * @brief Append one list to the end of another
 *
 * @param[in] list The list to which to append more objects
 * @param[in] more The list of objects to append
 *
 * Upon return @p list will have all of the objects from @p more
 * appended to its end.
 */
void list_concat(struct list * list, struct list * more);

/*!
 * @brief The direction in which to traverse the list during list_foreach()
 */
typedef enum {
    /*! @brief Traverse the list from front to back */
    LIST_FOREACH_DIR_FWD,
    /*! @brief Traverse the list from back to front */
    LIST_FOREACH_DIR_REV,
} list_foreach_dir_t;

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
int list_foreach(struct list * l,
                 cstl_visit_func_t * visit, void * priv,
                 list_foreach_dir_t dir);

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
void * list_find(const struct list * l,
                 const void * obj, cstl_compare_func_t * cmp, void * priv,
                 list_foreach_dir_t dir);

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
void list_clear(struct list * l, cstl_clear_func_t * clr);

/*!
 * @brief Swap the list objects at the two given locations
 *
 * @param[in,out] a A pointer to a list
 * @param[in,out] b A pointer to a(nother) list
 *
 * The lists at the given locations will be swapped such that upon return,
 * @p a will contain the list previously pointed to by @p b and vice versa.
 */
void list_swap(struct list * a, struct list * b);

/*! @} list */

#endif
