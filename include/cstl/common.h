/*!
 * @file
 */

#ifndef CSTL_COMMON_H
#define CSTL_COMMON_H

#define _CSTL_TOKCAT(A, B)      A ## B
#define CSTL_TOKCAT(A, B)       _CSTL_TOKCAT(A, B)

/*!
 * @defgroup lowlevel Low level containers
 */
/*!
 * @defgroup highlevel High level containers
 */

/*!
 * @brief Function type for comparing (in)equality of two objects
 *
 * @param[in] obj1 A pointer to an object
 * @param[in] obj2 A pointer to an object
 * @param[in] priv A pointer to private data belonging to the callee
 *
 * @retval <0 @p obj1 compares as less than @p obj2
 * @retval 0  @p obj1 compares as equal to @p obj2
 * @retval >0 @p obj1 compares as greater than @p obj2
 */
typedef int cstl_compare_func_t(
    const void * obj1, const void * obj2, void * priv);

/*!
 * @brief Type for visit callbacks from objects supporting foreach
 *
 * @param[in] obj A pointer to the object being visited
 * @param[in] priv A pointer to private data belonging to the callee
 *
 * For functions supporting the foreach functionality, this type of
 * function will be used when the callee is allowed to modify the
 * object being passed to the function.
 *
 * @retval 0 The foreach function should visit the next object
 * @retval Nonzero The foreach function should stop visiting objects
 */
typedef int cstl_visit_func_t(void * obj, void * priv);

/*!
 * @brief Type for @a const visit callbacks from objects supporting foreach
 *
 * @param[in] obj A pointer to the object being visited
 * @param[in] priv A pointer to private data belonging to the callee
 *
 * For functions supporting the foreach functionality, this type of
 * function will be used when the callee is @a not allowed to modify the
 * object being passed to the function.
 *
 * @retval 0 The foreach function should visit the next object
 * @retval Nonzero The foreach function should stop visiting objects
 */
typedef int cstl_const_visit_func_t(const void * obj, void * priv);

/*!
 * @brief Type for functions called to construct, clear, or destroy an object
 *
 * @param[in] obj A pointer to the object being visited
 * @param[in] priv A pointer to private data belonging to the callee
 *
 * Exactly how the callee should react to a call of this type is
 * context-specific. For construction, generally, the object has been
 * allocated, and the purpose of the call is to initialize the object.
 * The clear/destroy distinction is not always clear and depends very
 * much on the context. In some cases, it simply means that the memory
 * is no longer meant to hold the particular object, and the callee
 * should clear/free any data *held* by the object. In other cases,
 * it may mean to free the object itself (or both).
 */
typedef void cstl_xtor_func_t(void * obj, void * priv);

#include <stdint.h>
#include <string.h>

/*!
 * @brief Swap values at two memory locations via use of a third
 *
 * @param[in,out] x A pointer to the first value to be swapped
 * @param[in,out] y A pointer to the second value to be swapped
 * @param[out] t Scratch space that may be used to facilitate the swap
 * @param[in] sz The number of bytes pointed to by @p x, @p y, and @p t
 *
 * The values pointed to by @p x and @p y are swapped as if by @p memcpy().
 * The space at @p t may be used as a temporary/scratch space to facilitate
 * the swap. The space pointed to by @p x, @p y, and @p t must be at least
 * @p sz bytes in length.
 */
static inline
void cstl_swap(void * const x, void * const y, void * const t, const size_t sz)
{
#ifndef NO_DOC
#define EXCH(TYPE, A, B, T)                     \
    do {                                        \
        *(TYPE *)T = *(TYPE *)A;                \
        *(TYPE *)A = *(TYPE *)B;                \
        *(TYPE *)B = *(TYPE *)T;                \
    } while (0)
#endif

    switch (sz) {
    case sizeof(uint8_t):  EXCH(uint8_t, x, y, t);  break;
    case sizeof(uint16_t): EXCH(uint16_t, x, y, t); break;
    case sizeof(uint32_t): EXCH(uint32_t, x, y, t); break;
    case sizeof(uint64_t): EXCH(uint64_t, x, y, t); break;
    default:
        memcpy(t, x, sz);
        memcpy(x, y, sz);
        memcpy(y, t, sz);
        break;
    }

#undef EXCH
}

/*!
 * @brief Find the last (highest order) bit set
 *
 * @return Zero-based index of the highest order set bit
 * @retval -1 No bits are set, i.e. the input value is zero
 */
int cstl_fls(unsigned long);

#endif
