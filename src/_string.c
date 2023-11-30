/*!
 * @file
 */

#include "cstl/common.h"

#define STRV(NAME)              CSTL_TOKCAT(cstl_STRING, _##NAME)
#define STRF(NAME, ...)         STRV(NAME)(__VA_ARGS__)
#define STDSTRF(NAME, ...)      CSTL_TOKCAT(STDSTRPFX, NAME)(__VA_ARGS__)

#ifndef NO_DOC
#define cstl_STRING_char_t           STRV(char_t)
#endif

const cstl_STRING_char_t STRV(nul) = STRNUL;

/*! @private */
static inline cstl_STRING_char_t * STRF(
    __at, struct cstl_STRING * const s, const size_t i)
{
    return STRF(data, s) + i;
}

cstl_STRING_char_t * STRF(at, struct cstl_STRING * const s, const size_t i)
{
    if (i >= STRF(size, s)) {
        abort();
    }

    return STRF(__at, s, i);
}

const cstl_STRING_char_t * STRF(
    at_const, const struct cstl_STRING * const s, const size_t i)
{
    return STRF(at, (struct cstl_STRING *)s, i);
}

const cstl_STRING_char_t * STRF(str, const struct cstl_STRING * const s)
{
    const cstl_STRING_char_t * str = STRF(data, (struct cstl_STRING *)s);
    if (str == NULL) {
        str = &STRV(nul);
    }
    return str;
}

/*! @private */
static void STRF(__resize, struct cstl_STRING * const s, const size_t n)
{
    cstl_vector_resize(&s->v, n + 1);
    *STRF(__at, s, n) = STRV(nul);
}

void STRF(resize, struct cstl_STRING * const s, const size_t n)
{
    size_t sz = STRF(size, s);
    STRF(__resize, s, n);
    while (sz < n) {
        *STRF(__at, s, sz++) = STRV(nul);
    }
}

/*! @private */
static void STRF(prep_insert,
                 struct cstl_STRING * const s, const size_t pos,
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
                (size - pos) * sizeof(cstl_STRING_char_t));
    }
}

void STRF(insert_ch,
          struct cstl_STRING * const s, size_t idx,
          size_t cnt, const cstl_STRING_char_t ch)
{
    STRF(prep_insert, s, idx, cnt);
    while (cnt-- > 0) {
        *STRF(__at, s, idx++) = ch;
    }
}

void STRF(insert_str_n,
          struct cstl_STRING * const s, const size_t idx,
          const cstl_STRING_char_t * const str, const size_t len)
{
    STRF(prep_insert, s, idx, len);
    memcpy(STRF(__at, s, idx), str, len * sizeof(cstl_STRING_char_t));
}

ssize_t STRF(find_ch,
             const struct cstl_STRING * const s,
             const cstl_STRING_char_t c, const size_t pos)
{
    const cstl_STRING_char_t * const str = STRF(str, s);
    const size_t sz = STRF(size, s);

    const cstl_STRING_char_t * f;
    ssize_t i;

    if (pos >= sz) {
        abort();
    }

    i = -1;
    f = STDSTRF(chr, str + pos, c);
    if (f != NULL && f != str + sz) {
        i = ((uintptr_t)f - (uintptr_t)str) / sizeof(cstl_STRING_char_t);
    }

    return i;
}

ssize_t STRF(find_str,
             const struct cstl_STRING * const h,
             const cstl_STRING_char_t * const n, const size_t pos)
{
    const cstl_STRING_char_t * const str = STRF(str, h);

    const cstl_STRING_char_t * f;
    ssize_t i;

    if (pos >= STRF(size, h)) {
        abort();
    }

    i = -1;
    f = STDSTRF(str, str + pos, n);
    if (f != NULL) {
        i = ((uintptr_t)f - (uintptr_t)str) / sizeof(cstl_STRING_char_t);
    }

    return i;
}

/*! @private */
static void STRF(substr_prep,
                 const struct cstl_STRING * const s,
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
          const struct cstl_STRING * const s,
          const size_t idx, size_t len,
          struct cstl_STRING * const sub)
{
    STRF(substr_prep, s, idx, &len);
    STRF(__resize, sub, len);
    memcpy(STRF(__at, sub, 0),
           STRF(__at, (struct cstl_STRING *)s, idx),
           len * sizeof(cstl_STRING_char_t));
}

void STRF(erase, struct cstl_STRING * const s, const size_t idx, size_t len)
{
    const size_t size = STRF(size, s);
    STRF(substr_prep, s, idx, &len);
    memmove(STRF(__at, s, idx),
            STRF(__at, s, idx + len),
            (size - (idx + len)) * sizeof(cstl_STRING_char_t));
    STRF(__resize, s, size - len);
}

#undef cstl_STRING_char_t

#undef STDSTRF
#undef STRF
#undef STRV
