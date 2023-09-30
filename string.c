#include "string.h"

#include <stdlib.h>
#include <stdint.h>

void string_construct(struct string * const s)
{
    VECTOR_CONSTRUCT(&s->v, string_char_t);
    string_resize(s, 0);
}

static string_char_t * __string_at(struct string * const s, const size_t i)
{
    return (string_char_t *)vector_at(&s->v, i);
}

string_char_t * string_at(struct string * const s, const size_t i)
{
    if (i >= string_size(s)) {
        abort();
    }

    return __string_at(s, i);
}

const string_char_t * string_at_const(const struct string * const s,
                                      const size_t i)
{
    return string_at((struct string *)s, i);
}

static void __string_resize(struct string * const s, const size_t n)
{
    vector_resize(&s->v, n + 1);
    *__string_at(s, n) = '\0';
}

void string_resize(struct string * const s, const size_t n)
{
    const size_t sz = string_size(s);
    __string_resize(s, n);
    if (n > sz) {
        memset(__string_at(s, sz), '\0', (n - sz) *  sizeof(string_char_t));
    }
}

int string_compare_str(const struct string * const s,
                       const string_char_t * const str)
{
    return strcmp(string_str(s), str);
}

int string_compare(const struct string * const s1,
                   const struct string * const s2)
{
    return string_compare_str(s1, string_str(s2));
}

void string_insert(struct string * const s, const size_t idx,
                   const struct string * const s2)
{
    string_insert_strn(s, idx, string_str(s2), string_size(s2));
}

void string_insert_char(struct string * const s, const size_t idx,
                        const size_t cnt, const string_char_t ch)
{
    if (idx > string_size(s)) {
        abort();
    }

    if (cnt > 0) {
        __string_resize(s, string_size(s) + cnt);
        memmove(__string_at(s, idx + cnt),
                __string_at(s, idx),
                cnt * sizeof(string_char_t));
        memset(__string_at(s, idx), ch, cnt * sizeof(string_char_t));
    }
}

void string_insert_strn(struct string * const s, const size_t idx,
                        const string_char_t * const str, const size_t len)
{
    if (idx > string_size(s)) {
        abort();
    }

    if (len > 0) {
        const size_t size = string_size(s);
        __string_resize(s, size + len);
        memmove(__string_at(s, idx + len),
                __string_at(s, idx),
                (size - idx) * sizeof(string_char_t));
        memcpy(__string_at(s, idx), str, len * sizeof(string_char_t));
    }
}

void string_substr(const struct string * const s,
                   const size_t idx, size_t len,
                   struct string * const sub)
{
    const size_t size = string_size(s);

    if (idx >= size) {
        abort();
    }

    if (idx + len > size) {
        len = size - idx;
    }

    __string_resize(sub, len);
    memcpy(__string_at(sub, 0),
           __string_at((struct string *)s, idx),
           len * sizeof(string_char_t));
}

ssize_t string_find_char(const struct string * const s,
                         const string_char_t c, const size_t pos)
{
    const string_char_t * const str = string_str(s);
    const size_t sz = string_size(s);

    const string_char_t * f;
    ssize_t i;

    if (pos >= sz) {
        abort();
    }

    i = -1;
    f = strchr(str + pos, c);
    if (f != NULL && f != str + sz) {
        i = ((uintptr_t)f - (uintptr_t)str) / sizeof(string_char_t);
    }

    return i;
}

ssize_t string_find_str(const struct string * const h,
                        const char * const n, const size_t pos)
{
    const string_char_t * const str = string_str(h);

    const string_char_t * f;
    ssize_t i;

    if (pos >= string_size(h)) {
        abort();
    }

    i = -1;
    f = strstr(str + pos, n);
    if (f != NULL) {
        i = ((uintptr_t)f - (uintptr_t)str) / sizeof(string_char_t);
    }

    return i;
}

void string_erase(struct string * const s, const size_t idx, size_t len)
{
    const size_t size = string_size(s);

    if (idx >= size) {
        abort();
    }

    if (idx + len > size) {
        len = size - idx;
    }

    memmove(__string_at(s, idx),
            __string_at(s, idx + len),
            (size - (idx + len)) * sizeof(string_char_t));

    __string_resize(s, size - len);
}

void string_clear(struct string * const s)
{
    __string_resize(s, 0);
}

void string_destroy(struct string * const s)
{
    vector_destroy(&s->v);
}

#ifdef __cfg_test__
#include <check.h>

START_TEST(erase)
{
    struct string s;

    string_construct(&s);

    string_set_str(&s, "abc");
    string_erase(&s, 1, 1);
    ck_assert_str_eq(string_str(&s), "ac");

    string_set_str(&s, "abc");
    string_erase(&s, 1, 12);
    ck_assert_str_eq(string_str(&s), "a");

    string_destroy(&s);
}
END_TEST

START_TEST(substr)
{
    struct string s, sub;

    string_construct(&s);
    string_construct(&sub);

    string_set_str(&s, "abcdefg");
    string_substr(&s, 3, 3, &sub);
    ck_assert_str_eq(string_str(&sub), "def");

    string_set_str(&s, "abcdefg");
    string_substr(&s, 2, 12, &sub);
    ck_assert_str_eq(string_str(&sub), "cdefg");

    string_destroy(&sub);
    string_destroy(&s);
}
END_TEST

START_TEST(find)
{
    struct string s;

    string_construct(&s);

    string_set_str(&s, "abcdefghijk");

    ck_assert_int_eq(string_find_char(&s, 'd', 0), 3);
    ck_assert_int_eq(string_find_char(&s, 'e', 0), 4);
    ck_assert_int_eq(string_find_char(&s, '\0', 0), -1);
    ck_assert_int_eq(string_find_char(&s, 'd', 3), 3);
    ck_assert_int_eq(string_find_char(&s, 'e', 3), 4);
    ck_assert_int_eq(string_find_char(&s, 'z', 3), -1);

    ck_assert_int_eq(string_find_str(&s, "xyz", 0), -1);
    ck_assert_int_eq(string_find_str(&s, "abc", 0), 0);
    ck_assert_int_eq(string_find_str(&s, "ghikj", 0), -1);
    ck_assert_int_eq(string_find_str(&s, "efghij", 0), 4);
    ck_assert_int_eq(string_find_str(&s, "xyz", 4), -1);
    ck_assert_int_eq(string_find_str(&s, "abc", 4), -1);
    ck_assert_int_eq(string_find_str(&s, "ghikj", 4), -1);
    ck_assert_int_eq(string_find_str(&s, "efghij", 4), 4);

    string_destroy(&s);
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
    suite_add_tcase(s, tc);

    return s;
}

#endif
