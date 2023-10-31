/*!
 * @file
 */

#include "common.h"

#define STRV(NAME)              CSTL_TOKCAT(STRING, _##NAME)
#define STRF(NAME, ...)         STRV(NAME)(__VA_ARGS__)
#define STDSTRF(NAME, ...)      CSTL_TOKCAT(STDSTRPFX, NAME)(__VA_ARGS__)

#ifndef NO_DOC
#define STRING_char_t           STRV(char_t)
#endif

const STRING_char_t STRV(nul) = STRNUL;

/*! @private */
static inline STRING_char_t * STRF(
    __at, struct STRING * const s, const size_t i)
{
    return STRF(data, s) + i;
}

STRING_char_t * STRF(at, struct STRING * const s, const size_t i)
{
    if (i >= STRF(size, s)) {
        abort();
    }

    return STRF(__at, s, i);
}

const STRING_char_t * STRF(
    at_const, const struct STRING * const s, const size_t i)
{
    return STRF(at, (struct STRING *)s, i);
}

const STRING_char_t * STRF(str, const struct STRING * const s)
{
    const STRING_char_t * str = STRF(data, (struct STRING *)s);
    if (str == NULL) {
        str = &STRV(nul);
    }
    return str;
}

/*! @private */
static void STRF(__resize, struct STRING * const s, const size_t n)
{
    cstl_vector_resize(&s->v, n + 1);
    *STRF(__at, s, n) = STRV(nul);
}

void STRF(resize, struct STRING * const s, const size_t n)
{
    const size_t sz = STRF(size, s);
    STRF(__resize, s, n);
    if (n > sz) {
        memset(STRF(__at, s, sz),
               STRV(nul),
               (n - sz) *  sizeof(STRING_char_t));
    }
}

/*! @private */
static void STRF(prep_insert,
                 struct STRING * const s, const size_t pos,
                 const size_t len)
{
    if (pos > STRF(size, s)) {
        abort();
    }

    if (len > 0) {
        const size_t size = STRF(size, s);
        STRF(__resize, s, size + len);
        memmove(STRF(__at, s, pos + len),
                STRF(__at, s, pos),
                (size - pos) * sizeof(STRING_char_t));
    }
}

void STRF(insert_ch,
          struct STRING * const s, const size_t idx,
          const size_t cnt, const STRING_char_t ch)
{
    STRF(prep_insert, s, idx, cnt);
    memset(STRF(__at, s, idx), ch, cnt * sizeof(STRING_char_t));
}

void STRF(insert_str_n,
          struct STRING * const s, const size_t idx,
          const STRING_char_t * const str, const size_t len)
{
    STRF(prep_insert, s, idx, len);
    memcpy(STRF(__at, s, idx), str, len * sizeof(STRING_char_t));
}

ssize_t STRF(find_ch,
             const struct STRING * const s,
             const STRING_char_t c, const size_t pos)
{
    const STRING_char_t * const str = STRF(str, s);
    const size_t sz = STRF(size, s);

    const STRING_char_t * f;
    ssize_t i;

    if (pos >= sz) {
        abort();
    }

    i = -1;
    f = STDSTRF(chr, str + pos, c);
    if (f != NULL && f != str + sz) {
        i = ((uintptr_t)f - (uintptr_t)str) / sizeof(STRING_char_t);
    }

    return i;
}

ssize_t STRF(find_str,
             const struct STRING * const h,
             const STRING_char_t * const n, const size_t pos)
{
    const STRING_char_t * const str = STRF(str, h);

    const STRING_char_t * f;
    ssize_t i;

    if (pos >= STRF(size, h)) {
        abort();
    }

    i = -1;
    f = STDSTRF(str, str + pos, n);
    if (f != NULL) {
        i = ((uintptr_t)f - (uintptr_t)str) / sizeof(STRING_char_t);
    }

    return i;
}

/*! @private */
static void STRF(substr_prep,
                 const struct STRING * const s,
                 const size_t pos, size_t * const len)
{
    const size_t size = STRF(size, s);

    if (pos >= size) {
        abort();
    }

    if (pos + *len > size) {
        *len = size - pos;
    }
}

void STRF(substr,
          const struct STRING * const s,
          const size_t idx, size_t len,
          struct STRING * const sub)
{
    STRF(substr_prep, s, idx, &len);
    STRF(__resize, sub, len);
    memcpy(STRF(__at, sub, 0),
           STRF(__at, (struct STRING *)s, idx),
           len * sizeof(STRING_char_t));
}

void STRF(erase, struct STRING * const s, const size_t idx, size_t len)
{
    const size_t size = STRF(size, s);
    STRF(substr_prep, s, idx, &len);
    memmove(STRF(__at, s, idx),
            STRF(__at, s, idx + len),
            (size - (idx + len)) * sizeof(STRING_char_t));
    STRF(__resize, s, size - len);
}

#undef STRING_char_t

#undef STDSTRF
#undef STRF
#undef STRV
