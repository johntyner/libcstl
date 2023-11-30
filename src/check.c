// GCOV_EXCL_START
#include <stdlib.h>

#include "internal/check.h"

DECLARE_CK_JMP_BUF(signal);
void ck_handle_signal(const int signum,
                      siginfo_t * const si, void * const uc)
{
    (void)si; (void)uc;
    longjmp(CK_JMP_BUF(signal), signum);
}

int main(void)
{
    SRunner * sr;
    int failed;

    sr = srunner_create(suite_create(""));

#define SRUNNER_ADD_SUITE(SR, MODULE)                   \
    do {                                                \
        extern Suite * MODULE##_suite(void);            \
        srunner_add_suite(SR, MODULE##_suite());        \
    } while (0)

    SRUNNER_ADD_SUITE(sr, common);
    SRUNNER_ADD_SUITE(sr, memory);
    SRUNNER_ADD_SUITE(sr, bintree);
    SRUNNER_ADD_SUITE(sr, rbtree);
    SRUNNER_ADD_SUITE(sr, heap);
    SRUNNER_ADD_SUITE(sr, dlist);
    SRUNNER_ADD_SUITE(sr, slist);
    SRUNNER_ADD_SUITE(sr, hash);
    SRUNNER_ADD_SUITE(sr, vector);
    SRUNNER_ADD_SUITE(sr, string);
    SRUNNER_ADD_SUITE(sr, map);
    SRUNNER_ADD_SUITE(sr, array);
    SRUNNER_ADD_SUITE(sr, crc);

    srunner_run_all(sr, CK_ENV);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
// GCOV_EXCL_STOP
