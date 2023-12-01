/*!
 * @file
 */

#include <stdint.h>

#ifndef NO_DOC
#define REFLECT_PARTIAL(VAL, SHFT, MASK)                        \
    do {                                                        \
        MASK &= MASK >> SHFT;                                   \
        MASK |= MASK << (2 * SHFT);                             \
        VAL = ((VAL & MASK) << SHFT) | ((VAL >> SHFT) & MASK);  \
        SHFT >>= 1;                                             \
    } while (0)

/*
 * the compiler should be able to unroll these loops and
 * possibly even determine what the value of @m should be
 * at each iteration.
 *
 * complexity is log2(n) where n is the number of bits in the input
 */
#define REFLECT(WIDTH, VAL)                             \
    do {                                                \
        uint##WIDTH##_t m = ~((uint##WIDTH##_t)0);      \
        unsigned int s = (8 * sizeof(m)) / 2;           \
                                                        \
        while (s >= 8) {                                \
            REFLECT_PARTIAL(VAL, s, m);                 \
        }                                               \
                                                        \
        /*                                              \
         * at this point, the bytes are swapped. the    \
         * remainder of the function swaps the bits     \
         */                                             \
                                                        \
        do {                                            \
            REFLECT_PARTIAL(VAL, s, m);                 \
        } while (s > 0);                                \
    } while (0)
#endif

uint8_t cstl_reflect8(uint8_t x)
{
    REFLECT(8, x);
    return x;
}

uint16_t cstl_reflect16(uint16_t x)
{
    REFLECT(16, x);
    return x;
}

uint32_t cstl_reflect32(uint32_t x)
{
    REFLECT(32, x);
    return x;
}

uint64_t cstl_reflect64(uint64_t x)
{
    REFLECT(64, x);
    return x;
}

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

        for (i = 0, b = (8 * sizeof(x)) / 2; b > 0; b >>= 1) {
            const unsigned int s = b + i;
            const unsigned long m = ~((unsigned long)0) << s;

            if ((x & m) != 0) {
                i = s;
            }
        }
    }

    return i;
}

#ifdef __cfg_test__
// GCOV_EXCL_START
#include <check.h>
#include <stdlib.h>

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

START_TEST(reflect)
{
    ck_assert_uint_eq(0xed,
                      cstl_reflect8(0xb7));
    ck_assert_uint_eq(0xedb8,
                      cstl_reflect16(0x1db7));
    ck_assert_uint_eq(0xedb88320,
                      cstl_reflect32(0x04c11db7));
    ck_assert_uint_eq(0x82f63b78edb88320,
                      cstl_reflect64(0x04c11db71edc6f41));
}
END_TEST

Suite * bits_suite(void)
{
    Suite * const s = suite_create("bits");

    TCase * tc;

    tc = tcase_create("bits");
    tcase_add_test(tc, fls);
    tcase_add_test(tc, reflect);
    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
