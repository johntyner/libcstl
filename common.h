#ifndef CSTL_COMMON_H
#define CSTL_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

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
        memcpy(t, x, sz);
        memcpy(x, y, sz);
        memcpy(y, t, sz);
        break;
    }
#undef EXCH
}

int cstl_fls(unsigned long);

void cstl_abort(void);

#endif
