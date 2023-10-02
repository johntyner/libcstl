#include <stdlib.h>
#include <check.h>

Suite * common_suite(void);
Suite * memory_suite(void);
Suite * bintree_suite(void);
Suite * rbtree_suite(void);
Suite * heap_suite(void);
Suite * list_suite(void);
Suite * slist_suite(void);
Suite * vector_suite(void);
Suite * string_suite(void);

int main(void)
{
    SRunner * sr;
    int failed;

    sr = srunner_create(common_suite());
    srunner_add_suite(sr, memory_suite());
    srunner_add_suite(sr, bintree_suite());
    srunner_add_suite(sr, rbtree_suite());
    srunner_add_suite(sr, heap_suite());
    srunner_add_suite(sr, list_suite());
    srunner_add_suite(sr, slist_suite());
    srunner_add_suite(sr, vector_suite());
    srunner_add_suite(sr, string_suite());
    /*
     * add more suites with srunner_add_suite(sr, s);
     */

    srunner_run_all(sr, CK_ENV);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
