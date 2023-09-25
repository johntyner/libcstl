#include <stdlib.h>
#include <check.h>

Suite * bintree_suite(void);

int main(void)
{
    SRunner * sr;
    int failed;

    sr = srunner_create(bintree_suite());
    /*
     * add more suites with srunner_add_suite(sr, s);
     */

    srunner_run_all(sr, CK_ENV);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
