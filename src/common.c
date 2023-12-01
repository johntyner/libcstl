/*!
 * @file
 */

#include "cstl/common.h"

#ifdef __cfg_test__
// GCOV_EXCL_START
#include <check.h>
#include <stdlib.h>

START_TEST(swap)
{
#define SWAP_TEST(TYPE)                         \
    do {                                        \
        TYPE x = rand(), y = rand(), t;         \
        const TYPE a = x, b = y;                \
        cstl_swap(&x, &y, &t, sizeof(t));       \
        ck_assert_mem_eq(&a, &y, sizeof(a));    \
        ck_assert_mem_eq(&b, &x, sizeof(a));    \
    } while (0)

    SWAP_TEST(uint8_t);
    SWAP_TEST(uint16_t);
    SWAP_TEST(uint32_t);
    SWAP_TEST(uint64_t);

#undef SWAP_TEST
}

Suite * common_suite(void)
{
    Suite * const s = suite_create("common");

    TCase * tc;

    tc = tcase_create("common");
    tcase_add_test(tc, swap);
    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
