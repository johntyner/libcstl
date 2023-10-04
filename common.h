#ifndef CSTL_COMMON_H
#define CSTL_COMMON_H

#include "stdlib.h"

typedef int (cstl_compare_func_t)(const void *, const void *);
typedef int (cstl_visit_func_t)(void *, void *);
typedef int (cstl_const_visit_func_t)(const void *, void *);
typedef void (cstl_clear_func_t)(void *);

static inline void cstl_swap(void * const x, void * const y,
                             void * const t,
                             const size_t sz)
{
#define EXCH(TYPE, A, B, T)                     \
    do {                                        \
        *(TYPE *)T = *(TYPE *)A;                \
        *(TYPE *)A = *(TYPE *)B;                \
        *(TYPE *)B = *(TYPE *)T;                \
    } while (0)

    switch (sz) {
    case sizeof(uint8_t):  EXCH(uint8_t, x, y, t);  break;
    case sizeof(uint16_t): EXCH(uint16_t, x, y, t); break;
    case sizeof(uint32_t): EXCH(uint32_t, x, y, t); break;
    case sizeof(uint64_t): EXCH(uint64_t, x, y, t); break;
    default:
        cstl_memcpy(t, x, sz);
        cstl_memcpy(x, y, sz);
        cstl_memcpy(y, t, sz);
        break;
    }
#undef EXCH
}

int cstl_fls(unsigned long);

#endif
