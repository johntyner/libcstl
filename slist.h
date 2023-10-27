/*!
 * @file
 */

#ifndef CSTL_SLIST_H
#define CSTL_SLIST_H

/*!
 * @defgroup slist Singly-linked list
 * @brief A linked list allowing traversal in the forward direction
 */
/*!
 * @addtogroup slist
 * @{
 */

#include "common.h"

/*!
 * @brief Node to anchor an element within a list
 *
 * Users of the slist object declare this object within another
 * object as follows:
 * @code{.c}
 * struct object {
 *     ...
 *     struct slist_node node;
 *     ...
 * };
 * @endcode
 *
 * When calling slist_init(), the caller passes the offset of @p node
 * within their object as the @p off parameter of that function, e.g.
 * @code{.c}
 * offsetof(struct object, node)
 * @endcode
 */
struct slist_node
{
    /*! @privatesection */
    struct slist_node * n;
};

/*!
 * @brief List object
 *
 * Callers declare or allocate an object of this type to instantiate
 * a list. Users are encouraged to declare (and initialize) this
 * object with the DECLARE_SLIST() macro. Any other declaration or
 * allocation must be initialized via slist_init().
 */
struct slist
{
    /*! @privatesection */
    struct slist_node h;

    size_t count;
    size_t off;
};

/*!
 * @brief Constant initialization of a slist object
 *
 * @param TYPE The type of object that the list will hold
 * @param MEMB The name of the @p slist_node member within @p TYPE.
 *
 * @see slist_node for a description of the relationship between
 *                 @p TYPE and @p MEMB
 */
#define SLIST_INITIALIZER(TYPE, MEMB)           \
    {                                           \
        .h = {                                  \
            .n = NULL,                          \
        },                                      \
        .count = 0,                             \
        .off = offsetof(TYPE, MEMB),            \
    }
/*!
 * @brief (Statically) declare and initialize a list
 *
 * @param NAME The name of the variable being declared
 * @param TYPE The type of object that the list will hold
 * @param MEMB The name of the @p slist_node member within @p TYPE.
 *
 * @see slist_node for a description of the relationship between
 *                 @p TYPE and @p MEMB
 */
#define DECLARE_SLIST(NAME, TYPE, MEMB)                 \
    struct slist NAME = SLIST_INITIALIZER(TYPE, MEMB)

/*!
 * @brief Initialize a slist object
 *
 * @param[in,out] sl A pointer to the object to be initialized
 * @param[in] off The offset of the @p slist_node object within the
 *                object(s) that will be stored in the list
 */
static inline void slist_init(struct slist * const sl, const size_t off)
{
    sl->h.n = NULL;
    sl->count = 0;
    sl->off = off;
}

/*!
 * @brief Get the number of objects in the list
 *
 * @param[in] sl A pointer to the list
 *
 * @return The number of objects in the list
 */
static inline size_t slist_size(const struct slist * sl)
{
    return sl->count;
}

/*!
 * @brief Insert a new object into the list
 *
 * @param[in] sl A pointer to the list into which to insert the object
 * @param[in] before A pointer to an object already in the list
 * @param[in] obj A pointer to an object to insert into the list
 *
 * The new object will be inserted after the object pointed to by @p before
 */
void slist_insert_after(struct slist * sl, void * before, void * obj);
/*!
 * @brief Remove an object from the list
 *
 * @param[in] sl The list in which the object currently resides
 * @param[in] bef A pointer to the object in front of the object to be removed
 */
void * slist_erase_after(struct slist * sl, void * bef);

/*!
 * @brief Insert a new object at the front of the list
 *
 * @param[in] sl A pointer to a list
 * @param[in] obj A pointer to the object to be inserted
 */
void slist_push_front(struct slist * sl, void * obj);
/*!
 * @brief Remove the first item in the list and return it
 *
 * @param[in] sl A pointer to a list
 *
 * @return A pointer to the removed object
 * @retval NULL The list was empty and no object was removed
 */
void * slist_pop_front(struct slist * sl);

/*!
 * @brief Get a pointer to the first object in the list
 *
 * @param[in] sl A pointer to a list
 *
 * @return A pointer to the first object in the list
 * @retval NULL The list is empty
 */
void * slist_front(const struct slist * sl);

/*!
 * @brief Reverse the order of items in the list
 *
 * @param[in] sl A pointer to the list
 */
void slist_reverse(struct slist * sl);
/*!
 * @brief Sort the items in a list
 *
 * @param[in] sl A pointer to the list
 * @param[in] cmp A pointer to a function to compare objects
 * @param[in] priv A pointer to be passed to each invocation
 *                 of the comparison function
 *
 * The items are sorted from least to greatest, according to the provided
 * comparison function.
 */
void slist_sort(struct slist * sl, cstl_compare_func_t * cmp, void * priv);
/*!
 * @brief Append one list to the end of another
 *
 * @param[in] list The list to which to append more objects
 * @param[in] more The list of objects to append
 *
 * Upon return @p list will have all of the objects from @p more
 * appended to its end.
 */
void slist_concat(struct slist * list, struct slist * more);

/*!
 * @brief Call a user-supplied function for each object in a list
 *
 * @param[in] sl The list containing the objects to visit
 * @param[in] visit A pointer to the function to call for each object
 * @param[in] priv A pointer to be passed to each invocation of the function
 *
 * The user-supplied function must return 0 in order to continue visiting
 * objects in the list. The visiting of objects will stop at the first
 * occurrence of a non-zero return from the visit function.
 *
 * @return The value returned from the user-supplied function from the
 *         last object visited or 0
 */
int slist_foreach(struct slist * sl, cstl_visit_func_t * visit, void * priv);
/*!
 * @brief Remove objects from and reinitialize a list
 *
 * @param[in] sl The list to be cleared
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
void slist_clear(struct slist * sl, cstl_clear_func_t * clr);

/*!
 * @brief Swap the list objects at the two given locations
 *
 * @param[in,out] a A pointer to a list
 * @param[in,out] b A pointer to a(nother) list
 *
 * The lists at the given locations will be swapped such that upon return,
 * @p a will contain the list previously pointed to by @p b and vice versa.
 */
static inline void slist_swap(struct slist * const a,
                              struct slist * const b)
{
    struct slist t;
    cstl_swap(a, b, &t, sizeof(t));
}

/*! @} slist */

#endif
