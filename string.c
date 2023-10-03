#include "string.h"
#include "common.h"

#include <stdint.h>

static const string_char_t string_nul = '\0';

static string_char_t * __string_at(struct string * const s, const size_t i)
{
    return string_data(s) + i;
}

string_char_t * string_at(struct string * const s, const size_t i)
{
    if (i >= string_size(s)) {
        cstl_abort();
    }

    return __string_at(s, i);
}

const string_char_t * string_at_const(const struct string * const s,
                                      const size_t i)
{
    return string_at((struct string *)s, i);
}

const string_char_t * string_str(const struct string * const s)
{
    if (vector_size(&s->v) > 0) {
        return string_data((struct string *)s);
    } else {
        return &string_nul;
    }
}

static void __string_resize(struct string * const s, const size_t n)
{
    vector_resize(&s->v, n + 1);
    *__string_at(s, n) = string_nul;
}

void string_resize(struct string * const s, const size_t n)
{
    const size_t sz = string_size(s);
    __string_resize(s, n);
    if (n > sz) {
        memset(__string_at(s, sz),
               string_nul,
               (n - sz) *  sizeof(string_char_t));
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
    string_insert_str_n(s, idx, string_str(s2), string_size(s2));
}

void string_insert_ch(struct string * const s, const size_t idx,
                      const size_t cnt, const string_char_t ch)
{
    if (idx > string_size(s)) {
        cstl_abort();
    }

    if (cnt > 0) {
        __string_resize(s, string_size(s) + cnt);
        memmove(__string_at(s, idx + cnt),
                __string_at(s, idx),
                cnt * sizeof(string_char_t));
        memset(__string_at(s, idx), ch, cnt * sizeof(string_char_t));
    }
}

void string_insert_str_n(struct string * const s, const size_t idx,
                         const string_char_t * const str, const size_t len)
{
    if (idx > string_size(s)) {
        cstl_abort();
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
        cstl_abort();
    }

    if (idx + len > size) {
        len = size - idx;
    }

    __string_resize(sub, len);
    memcpy(__string_at(sub, 0),
           __string_at((struct string *)s, idx),
           len * sizeof(string_char_t));
}

ssize_t string_find_ch(const struct string * const s,
                       const string_char_t c, const size_t pos)
{
    const string_char_t * const str = string_str(s);
    const size_t sz = string_size(s);

    const string_char_t * f;
    ssize_t i;

    if (pos >= sz) {
        cstl_abort();
    }

    i = -1;
    f = strchr(str + pos, c);
    if (f != NULL && f != str + sz) {
        i = ((uintptr_t)f - (uintptr_t)str) / sizeof(string_char_t);
    }

    return i;
}

ssize_t string_find_str(const struct string * const h,
                        const string_char_t * const n, const size_t pos)
{
    const string_char_t * const str = string_str(h);

    const string_char_t * f;
    ssize_t i;

    if (pos >= string_size(h)) {
        cstl_abort();
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
        cstl_abort();
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
    vector_clear(&s->v);
}

#ifdef __cfg_test__
#include <check.h>

START_TEST(erase)
{
    struct string s;

    string_init(&s);

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
    struct string s, sub;

    string_init(&s);
    string_init(&sub);

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
    struct string s;

    string_init(&s);

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
    struct string s1, s2;

    string_init(&s1);
    string_init(&s2);

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
