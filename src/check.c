// GCOV_EXCL_START
#include <stdlib.h>
#include <check.h>

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

int main(void)
{
    SRunner * sr;
    int failed;

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
