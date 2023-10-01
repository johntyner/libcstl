#ifndef CSTL_STRING_H
#define CSTL_STRING_H

#include "vector.h"

#include <string.h>

typedef char string_char_t;

struct string
{
    struct vector v;
};

void string_construct(struct string *);

static inline size_t string_size(const struct string * const s)
{
    return vector_size(&s->v) - 1;
}

static inline size_t string_capacity(const struct string * const s)
{
    return vector_capacity(&s->v) - 1;
}

static inline void string_reserve(struct string * const s, const size_t n)
{
    vector_reserve(&s->v, n + 1);
}

void string_resize(struct string *, size_t);

string_char_t * string_at(struct string *, size_t);
const string_char_t * string_at_const(const struct string *, size_t);

string_char_t * string_data(struct string * const s)
{
    return vector_data(&s->v);
}

static inline const string_char_t * string_str(const struct string * const s)
{
    return vector_at_const(&s->v, 0);
}

int string_compare(const struct string *, const struct string *);
int string_compare_str(const struct string *, const string_char_t *);

void string_clear(struct string *);

void string_insert(struct string *, size_t, const struct string *);
void string_insert_char(struct string *, size_t, size_t, string_char_t);
void string_insert_strn(struct string *, size_t, const string_char_t *, size_t);
static inline void string_insert_str(struct string * const s, const size_t i,
                                     const string_char_t * const str)
{
    string_insert_strn(s, i, str, strlen(str));
}

static inline void string_append(struct string * const s1,
                                 const struct string * const s2)
{
    string_insert(s1, string_size(s1), s2);
}

static inline void string_append_char(struct string * const s,
                                      const size_t cnt, const string_char_t ch)
{
    string_insert_char(s, string_size(s), cnt, ch);
}

static inline void string_append_strn(struct string * const s,
                                      const string_char_t * const str,
                                      const size_t len)
{
    string_insert_strn(s, string_size(s), str, len);
}

static inline void string_append_str(struct string * const s,
                                     const string_char_t * const str)
{
    string_insert_str(s, string_size(s), str);
}

static inline void string_set_str(struct string * const s,
                                  const string_char_t * const str)
{
    string_clear(s);
    string_append_str(s, str);
}

void string_erase(struct string *, size_t, size_t);

void string_substr(const struct string *, size_t, size_t, struct string *);

ssize_t string_find_char(const struct string *, string_char_t, size_t);
ssize_t string_find_str(const struct string *, const string_char_t *, size_t);
static inline ssize_t string_find(const struct string * const h,
                                  const struct string * const n,
                                  const size_t p)
{
    return string_find_str(h, string_str(n), p);
}

void string_swap(struct string * const s1, struct string * const s2)
{
    vector_swap(&s1->v, &s2->v);
}

void string_destroy(struct string *);

#endif
