/*!
 * @file
 */

#ifndef CSTL_MEMORY_H
#define CSTL_MEMORY_H

/*!
 * @defgroup smartp Smartish pointers
 * @brief Non-automatic smart pointers
 *
 * In C++, memory can be allocated using unique_ptrs and shared_ptrs,
 * and the memory associated with them is freed automatically when
 * they go out of scope. The objects here attempt to mimic that behavior,
 * but in C, the caller is responsible for managing the lifetime of
 * the objects. The improvement here is that the caller must reset the
 * object(s) whenever they go out of scope.
 *
 * For example, without a smart pointer, the code may look something like:
 * @code{.c}
 * {
 *     void * data = malloc(len);
 *
 *     (void)somefunc(data);
 *
 *     // uh-oh, did somefunc() keep a pointer to data?
 *     // who is supposed to free it? this function
 *     // or somefunc()?
 * }
 * @endcode
 *
 * With a smart pointer, the above would look like:
 * @code{.c}
 * {
 *     DECLARE_CSTL_UNIQUE_PTR(data);
 *     cstl_unique_ptr_alloc(&data, len, NULL);
 *
 *     (void)somefunc(&data);
 *
 *     // it doesn't matter whether somefunc() kept
 *     // the pointer or not. this code must reset
 *     // the object before it goes out of scope.
 *
 *     cstl_unique_ptr_reset(&data);
 * }
 * @endcode
 */

#include "cstl/common.h"
#include <stdlib.h>

/*!
 * @defgroup guardp Guarded pointers
 * @ingroup smartp
 * @brief Object to guard against direct copying of pointers
 *
 * The smart pointer objects are defined in the header, and so the
 * programmer may directly copy those data structures via the @p = operator.
 * The cstl_guarded_ptr objects attempts to catch such uses, since the other
 * smart pointers depend on the non-direct copyability of their structures.
 */
/*!
 * @addtogroup guardp
 * @{
 */

/*!
 * @brief Initialize (at compile-time) a guarded pointer object
 *
 * @param[in] NAME The name of the object being initialized
 */
#define CSTL_GUARDED_PTR_INITIALIZER(NAME)      \
    {                                           \
        .ptr = NULL,                            \
        .self = &NAME,                          \
    }
/*!
 * @brief Declare and initialize a guarded pointer
 *
 * @param[in] NAME The name of the object being declared
 */
#define DECLARE_CSTL_GUARDED_PTR(NAME)          \
    struct cstl_guarded_ptr NAME =              \
        CSTL_GUARDED_PTR_INITIALIZER(NAME)

/*!
 * @brief Structure to hold a pointer and guard against its direct copying
 *
 * This object holds a pointer, but whenever that pointer is retrieved,
 * the code detects whether the object was directly copied via the @p =
 * operator. When such a use is detected, the code abort()s.
 */
struct cstl_guarded_ptr
{
    /*! @privatesection */
    void * self;
    void * ptr;
};

/*!
 * @brief Initialize a guarded pointer object to a specific pointer value
 *
 * @param[out] gp The guarded pointer to initialize
 * @param[in] ptr The pointer to store within the object
 *
 * The guarded pointer is (re)initialized regardless of its current state
 */
static inline void cstl_guarded_ptr_init_set(
    struct cstl_guarded_ptr * const gp,
    void * const ptr)
{
    gp->ptr = ptr;
    gp->self = gp;
}

/*!
 * @brief Initialize a guarded pointer object to NULL
 *
 * @param[out] gp The guarded pointer to initialize
 */
static inline void cstl_guarded_ptr_init(struct cstl_guarded_ptr * const gp)
{
    cstl_guarded_ptr_init_set(gp, NULL);
}

/*!
 * @brief Retrieve the stored pointer value
 *
 * If the object has been copied via the @p = operator, the attempt to
 * retrieve this function will cause an abort().
 *
 * @param[in] gp A pointer to the guarded pointer object
 *
 * @return The value of the pointer being guarded
 */
static inline void * cstl_guarded_ptr_get(
    const struct cstl_guarded_ptr * const gp)
{
    if (gp->self != gp) {
        abort();
    }
    return gp->ptr;
}

/*!
 * @brief Copy the cstl_guarded_ptr object to a new location
 *
 * The destination object is overwritten/(re)initialized, regardless of
 * its current state. If the source object has previously been copied via
 * the @p = operator, this function will cause an abort().
 *
 * @param[out] dst A pointer to the object to receive the copy
 * @param[in] src A pointer to the object to be copied
 */
static inline void cstl_guarded_ptr_copy(
    struct cstl_guarded_ptr * const dst,
    const struct cstl_guarded_ptr * const src)
{
    cstl_guarded_ptr_init_set(dst, cstl_guarded_ptr_get(src));
}

/*!
 * @brief Swap the pointers pointed to by the objects
 *
 * @param[in,out] a A pointer to a guarded pointer
 * @param[in,out] b A pointer to a(nother) guarded pointer
 */
static inline void cstl_guarded_ptr_swap(struct cstl_guarded_ptr * const a,
                                         struct cstl_guarded_ptr * const b)
{
    void * const t = cstl_guarded_ptr_get(a);
    cstl_guarded_ptr_init_set(a, cstl_guarded_ptr_get(b));
    cstl_guarded_ptr_init_set(b, t);
}

/*!
 * @}
 */
/*!
 * @defgroup uniquep Unique Pointers
 * @ingroup smartp
 * @brief Dynamically-allocated memory with a single owner
 *
 * The unique pointer is meant to have a single owner. It may be shared,
 * but that sharing is temporary, i.e. the sharing must end when the
 * callee returns/goes out of scope. Alternatively, the callee may take
 * ownership of the pointer by transferring it out of the caller's
 * unique pointer object and into its own.
 *
 * The unique pointer object manages the dynamically allocated memory
 * via the cstl_guarded_ptr object, and some functions may abort() if they
 * detect that that objects rules have been violated.
 */
/*!
 * @addtogroup uniquep
 * @{
 */

/*!
 * @brief Initialize (at compile time) a unique pointer
 *
 * @param[in] NAME The name of the object being initialized
 */
#define CSTL_UNIQUE_PTR_INITIALIZER(NAME)               \
    {                                                   \
        .gp = CSTL_GUARDED_PTR_INITIALIZER(NAME.gp),    \
        .clr = NULL,                                    \
    }
/*!
 * @brief Declare and initialize a unique pointer
 *
 * @param[in] NAME The name of the variable being declared
 */
#define DECLARE_CSTL_UNIQUE_PTR(NAME)           \
    cstl_unique_ptr_t NAME =                    \
        CSTL_UNIQUE_PTR_INITIALIZER(NAME)

/*!
 * @brief A pointer that has a single "owner"
 */
typedef struct
{
    /*! @privatesection */
    struct cstl_guarded_ptr gp;
    cstl_xtor_func_t * clr;
} cstl_unique_ptr_t;

/*!
 * @brief Initialize a unique pointer
 *
 * @param[out] up A pointer to a unique pointer object
 */
static inline void cstl_unique_ptr_init(cstl_unique_ptr_t * const up)
{
    cstl_guarded_ptr_init_set(&up->gp, NULL);
    up->clr = NULL;
}

/*!
 * @brief Dynamically allocate memory to be managed by the unique pointer
 *
 * The function allocates the requested number of bytes via malloc(),
 * and stores the resulting pointer within the unique pointer object.
 * The caller may provide a "clear" function that will be called prior
 * to the memory being freed whin the unique pointer is reset.
 *
 * @param[in] up A pointer to a unique pointer object
 * @param[in] len The number of bytes to allocate
 * @param[in] clr A pointer to a function to call when the memory is freed.
 *                This pointer may be NULL
 */
void cstl_unique_ptr_alloc(
    cstl_unique_ptr_t * up, size_t len, cstl_xtor_func_t * clr);

/*!
 * @brief Get the pointer managed by the unique pointer object
 *
 * @param[in] up A pointer to a unique pointer object
 *
 * @return The pointer managed by the unique pointer object
 * @retval NULL No pointer is managed by the unique pointer object
 */
static inline void * cstl_unique_ptr_get(const cstl_unique_ptr_t * const up)
{
    return cstl_guarded_ptr_get(&up->gp);
}

/*!
 * @brief Stop a unique pointer object from managing a pointer
 *
 * The managed pointer is returned, and the clear function is also
 * returned via the parameter, if non-NULL. Upon return, the object
 * does not manage any pointer and the caller is responsible for
 * calling the associated clear function and freeing the memory.
 *
 * @param[in] up A pointer to a unique pointer object
 * @param[in,out] clr A pointer to a function pointer to receive a pointer
 *                    to the associated clear function. This parameter may
 *                    be NULL
 *
 * @return The formerly managed pointer
 * @retval NULL The object was not managing a pointer
 */
static inline void * cstl_unique_ptr_release(
    cstl_unique_ptr_t * const up, cstl_xtor_func_t ** const clr)
{
    void * const p = cstl_unique_ptr_get(up);
    if (clr != NULL) {
        *clr = up->clr;
    }
    cstl_unique_ptr_init(up);
    return p;
}

/*!
 * @brief Swap the objects pointed to by the parameters
 *
 * @param[in,out] up1 A pointer to a unique pointer object
 * @param[in,out] up2 A pointer to a(nother) unique pointer object
 */
static inline void cstl_unique_ptr_swap(cstl_unique_ptr_t * const up1,
                                        cstl_unique_ptr_t * const up2)
{
    cstl_xtor_func_t * t;
    cstl_guarded_ptr_swap(&up1->gp, &up2->gp);
    cstl_swap(&up1->clr, &up2->clr, &t, sizeof(t));
}

/*!
 * @brief Free the memory managed by a unique pointer
 *
 * The managed pointer's clear function, if any, is called, and the
 * memory managed by the unique pointer is freed. Upon return, the
 * object no longer manages any memory and is in a newly initialized
 * state
 *
 * @param[in,out] up A pointer to the unique pointer object
 */
void cstl_unique_ptr_reset(cstl_unique_ptr_t * up);

/*!
 * @}
 */

/*!
 * @defgroup sharedp Shared Pointers
 * @ingroup smartp
 * @brief Reference-counted, dynamically-allocated memory
 *
 * The shared pointer object manages dynamically-allocated memory
 * by allowing it to be shared through the use of reference counting.
 * Multiple shared pointer objects may point to the same dynamically
 * allocated memory. The caller is responsible for mediating access
 * to the allocated memory, but the shared pointer object(s) will
 * manage the lifetime of that memory.
 *
 * @see weakp
 */
/*!
 * @addtogroup sharedp
 * @{
 */

/*!
 * @brief Compile-time initialization of a declared shared pointer
 *
 * @param[in] NAME The name of the variable being initialized
 */
#define CSTL_SHARED_PTR_INITIALIZER(NAME)                       \
    {                                                           \
        .data = CSTL_GUARDED_PTR_INITIALIZER(NAME.data),        \
    }
/*!
 * @brief Compile-time declaration and initialization of a shared pointer
 *
 * @param[in] NAME The name of the variable being declared
 */
#define DECLARE_CSTL_SHARED_PTR(NAME)                           \
    cstl_shared_ptr_t NAME = CSTL_SHARED_PTR_INITIALIZER(NAME)

/*!
 * @brief The shared pointer object
 */
typedef struct
{
    /*! @privatesection */
    struct cstl_guarded_ptr data;
} cstl_shared_ptr_t;

/*!
 * @brief Initialize a shared pointer object
 *
 * Upon return, the shared pointer manages no memory.
 *
 * @param[out] sp A pointer to a shared pointer object
 */
static inline void cstl_shared_ptr_init(cstl_shared_ptr_t * const sp)
{
    cstl_guarded_ptr_init_set(&sp->data, NULL);
}

/*!
 * @brief Dynamically allocated memory to be shared via the object
 *
 * The supplied shared pointer object must have already been initialized
 * and will be reset an preparation for the new allocation.
 *
 * @param[in,out] sp A pointer to the shared pointer object
 * @param[in] sz The number of bytes to allocate
 * @param[in] clr A function to be called when the allocated memory is freed.
 *                This pointer may be NULL
 */
void cstl_shared_ptr_alloc(
    cstl_shared_ptr_t * sp, size_t sz, cstl_xtor_func_t * clr);
/*!
 * @brief Return the number of shared pointer objects managing the memory
 *
 * The use count of the object may change via sharing through a different
 * managing object or through the introduction of a new owner via @ref weakp.
 *
 * @param[in] sp A pointer to a shared pointer object
 *
 * @return The number of shared pointers managing the underlying memory
 * @retval 0 The object does not manage any memory
 */
int cstl_shared_ptr_use_count(const cstl_shared_ptr_t *);
/*!
 * @brief Get a pointer to the memory managed by the object
 *
 * @return A pointer to the managed memory
 * @retval NULL No memory is managed by the object
 */
void * cstl_shared_ptr_get(const cstl_shared_ptr_t *);
/*!
 * @brief Create a new shared pointer object to manage the underlying memory
 *
 * @param[in] ex A pointer to the existing shared pointer
 * @param[in,out] n A pointer to a object with which to shared
 *                  the existing memory
 */
void cstl_shared_ptr_share(cstl_shared_ptr_t * ex, cstl_shared_ptr_t * n);

/*!
 * @brief Swap the memory managed by the two objects
 *
 * @param[in,out] sp1 A pointer to a shared pointer object
 * @param[in,out] sp2 A pointer to a(nother) shared pointer object
 */
static inline void cstl_shared_ptr_swap(cstl_shared_ptr_t * const sp1,
                                        cstl_shared_ptr_t * const sp2)
{
    cstl_guarded_ptr_swap(&sp1->data, &sp2->data);
}

/*!
 * @brief Stop managing the underlying memory via this object
 *
 * If this object is the last object managing the underlying memory,
 * the underlying memory will be free with its clear function, if
 * present, being called just prior to that.
 *
 * @param[in,out] sp A pointer to the shared object
 */
void cstl_shared_ptr_reset(cstl_shared_ptr_t * sp);

/*!
 * @}
 */
/*!
 * @defgroup weakp Weak Pointers
 * @ingroup smartp
 * @brief Non-"owning" reference to a cstl_shared_ptr
 *
 * Weak pointers point to memory managed by one or more shared pointer
 * objects, but do not "own" it. This means that a weak pointer may
 * be converted to a shared pointer while there is at least one other
 * valid shared pointer still managing the memory. If there are no other
 * shared pointer objects still managing the underlying memory, then any
 * attempt to convert a weak pointer to a shared pointer will fail.
 */
/*!
 * @addtogroup weakp
 * @{
 */

/*!
 * @brief Compile-time initialization of a weak pointer
 *
 * @param[in] NAME The name of the variable being initialized
 */
#define CSTL_WEAK_PTR_INITIALIZER(NAME)         \
    CSTL_SHARED_PTR_INITIALIZER(NAME)
/*!
 * @brief Compile-time declaration and initialization of a weak pointer
 *
 * @param[in] NAME The name of the variable being initialized
 */
#define DECLARE_CSTL_WEAK_PTR(NAME)                             \
    cstl_weak_ptr_t NAME = CSTL_WEAK_PTR_INITIALIZER(NAME)

/*!
 * @brief The weak pointer object
 */
typedef cstl_shared_ptr_t cstl_weak_ptr_t;

/*!
 * @brief Initialize a weak pointer object
 *
 * @param[out] wp A pointer to an uninitialized weak pointer object
 */
static inline void cstl_weak_ptr_init(cstl_weak_ptr_t * const wp)
{
    cstl_shared_ptr_init(wp);
}

/*!
 * @brief Create a weak pointer from a shared pointer
 *
 * The weak pointer must have already been initialized, and it will
 * be reset prior to becoming a reference to the shared memory.
 *
 * @param[in,out] wp A pointer to a weak pointer object
 * @param[in] sp A pointer to a shared pointer object
 */
void cstl_weak_ptr_from(cstl_weak_ptr_t * wp, const cstl_shared_ptr_t * sp);
/*!
 * @brief Convert a weak pointer to a shared pointer
 *
 * The weak pointer is not modified, but if the underlying memory
 * is still "live", the shared pointer will become an owning reference
 * to that memory.
 *
 * @param[in] wp A pointer to a weak pointer object
 * @param[in,out] sp A pointer to a shared pointer object
 */
void cstl_weak_ptr_lock(const cstl_weak_ptr_t * wp, cstl_shared_ptr_t * sp);

/*!
 * @brief Swap the memory managed by the two weak pointer objects
 *
 * @param[in,out] wp1 A pointer to a weak pointer object
 * @param[in,out] wp2 A pointer to a(nother) weak pointer object
 */
static inline void cstl_weak_ptr_swap(cstl_weak_ptr_t * const wp1,
                                      cstl_weak_ptr_t * const wp2)
{
    cstl_shared_ptr_swap(wp1, wp2);
}

/*!
 * @brief Drop the reference to the underlying managed memory
 *
 * The weak pointer does not "own" the managed memory, so if that memory
 * is still live, this function will not cause it to be freed.
 *
 * @param[in,out] wp A pointer to a weak pointer object
 */
void cstl_weak_ptr_reset(cstl_weak_ptr_t *);

/*!
 * @}
 */

#endif
