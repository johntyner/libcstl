/*!
 * @file
 */

#include "cstl/common.h"

int cstl_fls(const unsigned long x)
{
    int i = -1;

    if (x != 0) {
        unsigned int b;

        /*
         * this loop performs a binary search by setting
         * the upper half of the bits and determining if
         * a bit in the input is set in that half.
         *
         * each iteration of the loop reduces the size of
         * the mask by half and either moves it to the upper
         * half of the previous half (if a set bit was found)
         * or the upper half of the previous unset half (if
         * a set bit was not found).
         *
         * the runtime of the algorithm is log2(n) where n
         * is the total number of bits in the input value.
         */

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