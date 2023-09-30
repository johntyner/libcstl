#include "common.h"

#include <stdint.h>
#include <string.h>

int cstl_fls(unsigned long x)
{
    int i = -1;

    if (x != 0) {
        unsigned int b;

        for (i = 0, b = (8 * sizeof(x)) / 2; b != 0; b /= 2) {
            const unsigned int s = b + i;
            unsigned long m;

            m = ~0;
            m <<= s;

            if ((x & m) != 0) {
                i = s;
            }
        }
    }

    return i;
}

void cstl_swap(void * const x, void * const y,
               void * const t,
               const size_t sz)
{
#define EXCH(TYPE, A, B)                        \
    do {                                        \
        const TYPE c = *(TYPE *)A;              \
        *(TYPE *)A = *(TYPE *)B;                \
        *(TYPE *)B = c;                         \
    } while (0)

    switch (sz) {
    case sizeof(uint8_t):  EXCH(uint8_t, x, y);  break;
    case sizeof(uint16_t): EXCH(uint16_t, x, y); break;
    case sizeof(uint32_t): EXCH(uint32_t, x, y); break;
    case sizeof(uint64_t): EXCH(uint64_t, x, y); break;
    default:
        memcpy(t, x, sz);
        memcpy(x, y, sz);
        memcpy(y, t, sz);
        break;
    }
}

#ifdef __cfg_test__
#include <check.h>

START_TEST(fls)
{
    ck_assert_int_eq(cstl_fls(0), -1);
    ck_assert_int_eq(cstl_fls(1), 0);
    ck_assert_int_eq(cstl_fls(3), 1);
    ck_assert_int_eq(cstl_fls(3 << 16), 17);
    ck_assert_int_eq(cstl_fls(~0UL), 8 * sizeof(unsigned long) - 1);
    ck_assert_int_eq(cstl_fls(0x5a5a5a5a), 30);
}
END_TEST

Suite * common_suite(void)
{
    Suite * const s = suite_create("common");

    TCase * tc;

    tc = tcase_create("common");
    tcase_add_test(tc, fls);
    suite_add_tcase(s, tc);

    return s;
}

#endif
