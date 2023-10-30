/*!
 * @file
 *
 * Much like string.h, this file serves only to include the actual
 * implementation file after setting certain macros to enable the
 * "templatization" in that file to work. This file mus set the
 * @p %STRING and @p %STDSTRPFX macros as described in string.h. In
 * addition it must set @p %STRNUL to the NUL character as represented
 * by the string's character type.
 *
 * This file also contains the unit tests for the string object(s)
 *
 * @see string.h for a description of necessary macros
 * @see _string.c for the templatized string object implementation
 */

#include "string.h"

#include <stdlib.h>
#include <string.h>

#ifndef NO_DOC
#define STRNUL                  '\0'
#define STRING                  string
#define STDSTRPFX               str
#include "_string.c"
#undef STDSTRPFX
#undef STRING
#undef STRNUL
#endif

#ifndef NO_DOC
#define STRNUL                  L'\0'
#define STRING                  wstring
#define STDSTRPFX               wcs
#include "_string.c"
#undef STDSTRPFX
#undef STRING
#undef STRNUL
#endif

#ifdef __cfg_test__
#include <check.h>

START_TEST(erase)
{
    DECLARE_STRING(string, s);

    string_set_str(&s, "abc");
    string_erase(&s, 1, 1);
    ck_assert_str_eq(string_str(&s), "ac");

    string_set_str(&s, "abc");
    string_erase(&s, 1, 12);
    ck_assert_str_eq(string_str(&s), "a");

    string_clear(&s);
}
END_TEST

START_TEST(substr)
{
    DECLARE_STRING(string, s);
    DECLARE_STRING(string, sub);

    string_set_str(&s, "abcdefg");
    string_substr(&s, 3, 3, &sub);
    ck_assert_str_eq(string_str(&sub), "def");

    string_set_str(&s, "abcdefg");
    string_substr(&s, 2, 12, &sub);
    ck_assert_str_eq(string_str(&sub), "cdefg");

    string_clear(&sub);
    string_clear(&s);
}
END_TEST

START_TEST(find)
{
    DECLARE_STRING(string, s);

    string_set_str(&s, "abcdefghijk");

    ck_assert_int_eq(string_find_ch(&s, 'd', 0), 3);
    ck_assert_int_eq(string_find_ch(&s, 'e', 0), 4);
    ck_assert_int_eq(string_find_ch(&s, string_nul, 0), -1);
    ck_assert_int_eq(string_find_ch(&s, 'd', 3), 3);
    ck_assert_int_eq(string_find_ch(&s, 'e', 3), 4);
    ck_assert_int_eq(string_find_ch(&s, 'z', 3), -1);

    ck_assert_int_eq(string_find_str(&s, "xyz", 0), -1);
    ck_assert_int_eq(string_find_str(&s, "abc", 0), 0);
    ck_assert_int_eq(string_find_str(&s, "ghikj", 0), -1);
    ck_assert_int_eq(string_find_str(&s, "efghij", 0), 4);
    ck_assert_int_eq(string_find_str(&s, "xyz", 4), -1);
    ck_assert_int_eq(string_find_str(&s, "abc", 4), -1);
    ck_assert_int_eq(string_find_str(&s, "ghikj", 4), -1);
    ck_assert_int_eq(string_find_str(&s, "efghij", 4), 4);

    string_clear(&s);
}
END_TEST

START_TEST(swap)
{
    DECLARE_STRING(string, s1);
    DECLARE_STRING(string, s2);

    string_set_str(&s1, "hello");
    string_set_str(&s2, "world");

    string_swap(&s1, &s2);

    ck_assert_str_eq(string_str(&s1), "world");
    ck_assert_str_eq(string_str(&s2), "hello");

    string_clear(&s2);
    string_clear(&s1);
}
END_TEST

Suite * string_suite(void)
{
    Suite * const s = suite_create("string");

    TCase * tc;

    tc = tcase_create("string");
    tcase_add_test(tc, erase);
    tcase_add_test(tc, substr);
    tcase_add_test(tc, find);
    tcase_add_test(tc, swap);
    suite_add_tcase(s, tc);

    return s;
}

#endif
