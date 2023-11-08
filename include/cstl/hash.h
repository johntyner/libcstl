/*!
 * @file
 */

#ifndef CSTL_HASH_H
#define CSTL_HASH_H

/*!
 * @defgroup hash Hash table
 * @ingroup lowlevel
 * @brief A hash table utilizing separate chaining for collision resolution
 */
/*!
 * @addtogroup hash
 * @{
 */

#include "cstl/common.h"

#include <stdbool.h>

/*!
 * @brief Function type for hashing a key into a bucket
 *
 * Each element inserted into the hash has an integer associated
 * with it, aka a "key". This key must be "hashed" in order to fit
 * into the hash table. The function that does this hashing must
 * be of this type.
 *
 * @param[in] k The key to be hashed
 * @param[in] m The size of the hash table
 *
 * @return A value in the range [0, m)
 */
typedef size_t cstl_hash_func_t(unsigned long k, size_t m);

/*!
 * @brief Node to anchor an element within a hash
 *
 * Users of the hash object declare this object within another
 * object as follows:
 * @code{.c}
 * struct object {
 *     ...
 *     struct cstl_hash_node hnode;
 *     ...
 * };
 * @endcode
 *
 * When calling cstl_hash_init(), the caller passes the offset of @p hnode
 * within their object as the @p off parameter of that function, e.g.
 * @code{.c}
 * offsetof(struct object, hnode)
 * @endcode
 */
struct cstl_hash_node
{
    /*! @privatesection */
    unsigned long key;
    struct cstl_hash_node * next;

    /*
     * There's an optimization to be made here: when
     * rehashing, it's possible that the node is rehashed
     * but ends up in a bucket that has not yet been cleaned.
     * if the *node* can be marked as having been cleaned,
     * it can be skipped when the containing bucket is cleaned.
     *
     * whether or not carrying around that bit of state in
     * this structure (for every node!) and the extra logic to
     * accommodate the optimization are worthwhile is an open
     * question. prevailing wisdom suggests that it is not.
     */
};

/*!
 * @brief Hash object
 *
 * Callers declare or allocate an object of this type to instantiate
 * a hash. Users are encouraged to declare (and initialize) this
 * object with the DECLARE_CSTL_HASH() macro. Any other declaration or
 * allocation must be initialized via cstl_hash_init().
 */
struct cstl_hash
{
    /*! @privatesection */
    struct {
        /*! @privatesection */
        struct cstl_hash_bucket {
            /* list of nodes in the bucket */
            struct cstl_hash_node * n;
            /*
             * clean state of the bucket.
             * if cst matches the outer cst, then
             * the bucket is clean.
             */
            bool cst;
        } * at;

        /*
         * number of buckets in use and number of
         * available buckets
         */
        size_t count, capacity;
        cstl_hash_func_t * hash;
        /*
         * desired clean state. when the table is
         * resized, this bit flips which causes all
         * of the buckets to be regarded as dirty
         */
        bool cst;

        struct {
            /*
             * during a resize/rehash, @count are @hash
             * are the desired parameters of the table
             * after the rehash is complete. @clean is
             * the index of the next bucket to be cleaned
             * during the incremental sweep
             *
             * whether a rehash is in process is determined
             * by whether or not @hash is NULL
             */
            size_t count, clean;
            cstl_hash_func_t * hash;
        } rh;
    } bucket;

    size_t count;
    size_t off;
};

/*!
 * @brief Constant initialization of a hash object
 *
 * @param TYPE The type of object that the hash will hold
 * @param MEMB The name of the @p cstl_hash_node member within @p TYPE.
 *
 * @see cstl_hash_node for a description of the relationship between
 *                     @p TYPE and @p MEMB
 */
#define CSTL_HASH_INITIALIZER(TYPE, MEMB)       \
    {                                           \
    .bucket = {                                 \
        .at = NULL,                             \
        .count = 0,                             \
        .capacity = 0,                          \
        .hash = NULL,                           \
        .cst = false,                           \
        .rh = {                                 \
            .hash = NULL,                       \
        },                                      \
    },                                          \
    .count = 0,                                 \
    .off = offsetof(TYPE, MEMB),                \
}
/*!
 * @brief (Statically) declare and initialize a hash
 *
 * @param NAME The name of the variable being declared
 * @param TYPE The type of object that the hash will hold
 * @param MEMB The name of the @p cstl_hash_node member within @p TYPE.
 *
 * @see cstl_hash_node for a description of the relationship between
 *                     @p TYPE and @p MEMB
 */
#define DECLARE_CSTL_HASH(NAME, TYPE, MEMB)                     \
    struct cstl_hash NAME = CSTL_HASH_INITIALIZER(TYPE, MEMB)

/*!
 * @brief Initialize a hash object
 *
 * @param[in,out] h A pointer to the object to be initialized
 * @param[in] off The offset of the @p cstl_hash_node object within the
 *                object(s) that will be stored in the hash
 *
 * @note The hash object is not ready for use until it has been
 *       resized via the cstl_hash_resize() function
 */
static inline void cstl_hash_init(
    struct cstl_hash * const h, const size_t off)
{
    h->bucket.at = NULL;
    h->bucket.count = 0;
    h->bucket.capacity = 0;
    h->bucket.cst = 0;
    h->bucket.hash = NULL;

    h->bucket.rh.hash = NULL;

    h->count = 0;
    h->off = off;
}

/*!
 * @brief Get the number of objects in the hash
 *
 * @param[in] h A pointer to the hash
 *
 * @return The number of objects in the hash
 */
size_t cstl_hash_size(const struct cstl_hash * const h)
{
    return h->count;
}

/*!
 * @brief Get the average number of nodes per bucket
 *
 * @return The average number of nodes per bucket, i.e. the total number
 *         of nodes divided by the number of buckets
 */
float cstl_hash_load(const struct cstl_hash * const h)
{
    size_t count = h->bucket.count;
    if (h->bucket.rh.hash != NULL) {
        count = h->bucket.rh.count;
    }
    return (float)h->count / count;
}

/*!
 * @brief Free memory associated with excess buckets
 *
 * As the table is resized, the number of buckets is increased or
 * decreased. When decreasing, the memory associated with unused
 * buckets is not released. This function releases that memory. The
 * number of buckets is not changed.
 *
 * @param[in,out] h A pointer to a hash object
 */
void cstl_hash_shrink_to_fit(struct cstl_hash * h);

/*!
 * @brief Resize the hash table
 *
 * @param[in,out] h A pointer to the hash object
 * @param[in] n The desired number of buckets in the table
 * @param[in] f The function used by this hash object to hash keys. If this
 *              parameter is NULL, the existing hash function will be reused.
 *              If there is no existing hash function (because the hash object
 *              is in an initialized state), the cstl_hash_mul() function will
 *              be used.
 *
 * If @p n is zero, this function does nothing. Once this function has been
 * called, cstl_hash_clear() must be called to re/de-initialize the object.
 * If the function fails, the original hash object is undisturbed.
 */
void cstl_hash_resize(struct cstl_hash * h, size_t n, cstl_hash_func_t * f);

/*!
 * @brief Rehash the hash table
 *
 * After resizing or changing the hash function for the table, all
 * nodes in the table must be moved to new buckets. This is done
 * incrementally, spreading the time cost over multiple operations on
 * the table. However, this function may be used to force the operation
 * to run to completion immediately.
 *
 * If a rehash is not currently in progress, the function returns immediately.
 *
 * @param[in,out] h A pointer to the hash object
 */
void cstl_hash_rehash(struct cstl_hash * h);

/*!
 * @brief Insert an item into the hash
 *
 * @param[in] h A pointer to the hash object
 * @param[in] k The key associated with the object being inserted
 * @param[in] e A pointer to the object to insert
 *
 * If the caller maintains a pointer to the object or otherwise gets
 * a pointer via cstl_hash_find() or cstl_hash_foreach(), the caller may modify
 * the object as desired. However, the key associated with the object
 * must not be changed.
 */
void cstl_hash_insert(struct cstl_hash * h, unsigned long k, void * e);

/*!
 * @brief Lookup/find a previously inserted object in the hash
 *
 * @param[in] h A pointer to the hash object
 * @param[in] k The key associated with the object being sought
 * @param[in] visit A pointer to a function that will be called for
 *                  each object with a matching key. The called function
 *                  should return a non-zero value when the desired object
 *                  is found. The pointer may be NULL, in which case, the
 *                  first object with a matching key will be returned.
 * @param[in] priv A pointer, belonging to the caller, that will be passed
 *                 to each invocation of the @p visit function
 *
 * @return A pointer to the object that was found
 * @retval NULL No object with a matching key was found, or the @p visit
 *              function did not identify a matching object
 */
void * cstl_hash_find(struct cstl_hash * h, unsigned long k,
                      cstl_const_visit_func_t * visit, void * priv);

/*!
 * @brief Remove an object from the hash
 *
 * @param[in] h A pointer to the hash object
 * @param[in] e A pointer to the object to be removed. This pointer must
 *              be to the *actual* object to be removed, not just to an
 *              object that would compare as equal
 */
void cstl_hash_erase(struct cstl_hash * h , void * e);

/*!
 * @brief Visit each object within a hash table
 *
 * @param[in] h A pointer to the hash object
 * @param[in] visit A function to be called for each object in the table. The
 *                  function should return zero to continue visiting objects
 *                  or a non-zero value to terminate the foreach function.
 *                  The visit function may alter the object (but not the key)
 *                  and/or remove the *current* object from the table.
 * @param[in] priv A pointer, belonging to the caller, that will be passed
 *                 to each invocation of the @p visit function
 *
 * @note This function will force an in-progress rehash to complete. Consider
 * cstl_hash_foreach_const() if the visited nodes do not need to be modified
 * or removed.
 *
 * @return The value returned by the last invocation of @p visit or 0
 */
int cstl_hash_foreach(struct cstl_hash * h,
                      cstl_visit_func_t * visit, void * priv);

/*!
 * @brief Visit each object within a hash table
 *
 * @param[in] h A pointer to the hash object
 * @param[in] visit A function to be called for each object in the table. The
 *                  function should return zero to continue visiting objects
 *                  or a non-zero value to terminate the foreach function.
 * @param[in] priv A pointer, belonging to the caller, that will be passed
 *                 to each invocation of the @p visit function
 *
 * @return The value returned by the last invocation of @p visit or 0
 */
int cstl_hash_foreach_const(const struct cstl_hash * h,
                            cstl_const_visit_func_t * visit, void * priv);

/*!
 * @brief Remove all elements from the hash
 *
 * @param[in] h A pointer to the hash
 * @param[in] clr A pointer to a function to be called for each element in
 *                the hash. The function may be NULL.
 *
 * All elements are removed from the hash and the @p clr function is
 * called for each element that was in the hash. The order in which
 * the elements are removed and @p clr is called is not specified, but
 * the callee may take ownership of an element at the time that @p clr
 * is called for that element and not before.
 *
 * Upon return from this function, the hash contains no elements, and
 * is as it was immediately after being initialized. No further operations
 * on the hash are necessary to make it ready to go out of scope or be
 * destroyed.
 */
void cstl_hash_clear(struct cstl_hash * h, cstl_xtor_func_t * clr);

/*!
 * @brief Swap the hash objects at the two given locations
 *
 * @param[in,out] a A pointer to a hash
 * @param[in,out] b A pointer to a(nother) hash
 *
 * The hashes at the given locations will be swapped such that upon return,
 * @p a will contain the hash previously pointed to by @p b and vice versa.
 */
static inline void cstl_hash_swap(struct cstl_hash * const a,
                                  struct cstl_hash * const b)
{
    struct cstl_hash t;
    cstl_swap(a, b, &t, sizeof(t));
}

/*!
 * @name Built-in key hashing functions
 * @{
 */

/*!
 * @brief Hash by division
 *
 * The key is hashed by dividing it by the number of buckets in the
 * table and returning the remainder.
 *
 * @param[in] k The key to be hashed
 * @param[in] m The size of the hash table
 *
 * @return A value in the range [0, m)
 */
size_t cstl_hash_div(unsigned long k, size_t m);

/*!
 * @brief Hash by multiplication
 *
 * The key is hashed by multiplying it by a value @p phi and then
 * multiplying the fractional portion of that result by @p m. In
 * this case, the value @p phi is the golden ratio (1.618034), as
 * suggested by Knuth.
 *
 * @param[in] k The key to be hashed
 * @param[in] m The size of the hash table
 *
 * @return A value in the range [0, m)
 */
size_t cstl_hash_mul(unsigned long k, size_t m);
/*!
 * @}
 */

/*!
 * @}
 */

#endif
