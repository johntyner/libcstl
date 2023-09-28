#include "list.h"

#ifdef __cfg_test__
#include <check.h>

Suite * list_suite(void)
{
    Suite * const s = suite_create("list");

    TCase * tc;

    tc = tcase_create("list");
    /* tcase_add_test(tc, fill); */
    suite_add_tcase(s, tc);

    return s;
}

#endif
