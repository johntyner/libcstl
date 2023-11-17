// GCOV_EXCL_START
#include <stdlib.h>
#include <signal.h>

#include "cstl/internal/check.h"

Suite * common_suite(void);
Suite * memory_suite(void);
Suite * bintree_suite(void);
Suite * rbtree_suite(void);
Suite * heap_suite(void);
Suite * dlist_suite(void);
Suite * slist_suite(void);
Suite * hash_suite(void);
Suite * vector_suite(void);
Suite * string_suite(void);
Suite * map_suite(void);
Suite * array_suite(void);

/*
 * by initializing the jmp_buf, we ensure that if the signal
 * is raised via the ck_raise_signal function, the code will
 * return here if the test wasn't expecting it.
 */
#define CK_JMP_BUF_INIT(NAME)                           \
    do {                                                \
        const int res = setjmp(CK_JMP_BUF(NAME));       \
        if (res != 0) {                                 \
            ck_abort_msg(                               \
                "received unexpected signal %d\n",      \
                res);                                   \
        }                                               \
    } while (0)

DECLARE_CK_JMP_BUF(signal);

int main(void)
{
    SRunner * sr;
    int failed;

    CK_JMP_BUF_INIT(signal);

    sr = srunner_create(suite_create(""));
    srunner_add_suite(sr, common_suite());
    srunner_add_suite(sr, memory_suite());
    srunner_add_suite(sr, bintree_suite());
    srunner_add_suite(sr, rbtree_suite());
    srunner_add_suite(sr, heap_suite());
    srunner_add_suite(sr, dlist_suite());
    srunner_add_suite(sr, slist_suite());
    srunner_add_suite(sr, hash_suite());
    srunner_add_suite(sr, vector_suite());
    srunner_add_suite(sr, string_suite());
    srunner_add_suite(sr, map_suite());
    srunner_add_suite(sr, array_suite());
    /*
     * add more suites with srunner_add_suite(sr, s);
     */

    srunner_run_all(sr, CK_ENV);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
// GCOV_EXCL_STOP
