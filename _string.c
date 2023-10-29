/*!
 * @file
 */

/*!
 * @addtogroup string
 * @{
 */

#define _STRCAT(A, B)           A ## B
#define STRCAT(A, B)            _STRCAT(A, B)

#define STRV(NAME)              STRCAT(STRING, _##NAME)
#define STRF(NAME, ...)         STRV(NAME)(__VA_ARGS__)
#define STDSTRF(NAME, ...)      STRCAT(STDSTRPFX, NAME)(__VA_ARGS__)

#ifndef NO_DOC
#define STRING_char_t           STRV(char_t)
#endif

/*! @private */
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
        cstl_abort();
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
    if (vector_size(&s->v) > 0) {
        return STRF(data, (struct STRING *)s);
    } else {
        return &STRV(nul);
    }
}

/*! @private */
static void STRF(__resize, struct STRING * const s, const size_t n)
{
    vector_resize(&s->v, n + 1);
    *STRF(__at, s, n) = STRV(nul);
}

void STRF(resize, struct STRING * const s, const size_t n)
{
    const size_t sz = STRF(size, s);
    STRF(__resize, s, n);
    if (n > sz) {
        cstl_memset(STRF(__at, s, sz),
                    STRV(nul),
                    (n - sz) *  sizeof(STRING_char_t));
    }
}

int STRF(compare_str,
            const struct STRING * const s,
            const STRING_char_t * const str)
{
    return STDSTRF(cmp, STRF(str, s), str);
}

int STRF(compare,
            const struct STRING * const s1,
            const struct STRING * const s2)
{
    return STRF(compare_str, s1, STRF(str, s2));
}

void STRF(insert,
             struct STRING * const s, const size_t idx,
             const struct STRING * const s2)
{
    STRF(insert_str_n, s, idx, STRF(str, s2), STRF(size, s2));
}

void STRF(insert_ch,
             struct STRING * const s, const size_t idx,
             const size_t cnt, const STRING_char_t ch)
{
    if (idx > STRF(size, s)) {
        cstl_abort();
    }

    if (cnt > 0) {
        STRF(__resize, s, STRF(size, s) + cnt);
        cstl_memmove(STRF(__at, s, idx + cnt),
                     STRF(__at, s, idx),
                     cnt * sizeof(STRING_char_t));
        cstl_memset(STRF(__at, s, idx), ch, cnt * sizeof(STRING_char_t));
    }
}

void STRF(insert_str_n,
             struct STRING * const s, const size_t idx,
             const STRING_char_t * const str, const size_t len)
{
    if (idx > STRF(size, s)) {
        cstl_abort();
    }

    if (len > 0) {
        const size_t size = STRF(size, s);
        STRF(__resize, s, size + len);
        cstl_memmove(STRF(__at, s, idx + len),
                     STRF(__at, s, idx),
                     (size - idx) * sizeof(STRING_char_t));
        cstl_memcpy(STRF(__at, s, idx), str, len * sizeof(STRING_char_t));
    }
}

void STRF(substr,
             const struct STRING * const s,
             const size_t idx, size_t len,
             struct STRING * const sub)
{
    const size_t size = STRF(size, s);

    if (idx >= size) {
        cstl_abort();
    }

    if (idx + len > size) {
        len = size - idx;
    }

    STRF(__resize, sub, len);
    cstl_memcpy(STRF(__at, sub, 0),
                STRF(__at, (struct STRING *)s, idx),
                len * sizeof(STRING_char_t));
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
        cstl_abort();
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
        cstl_abort();
    }

    i = -1;
    f = STDSTRF(str, str + pos, n);
    if (f != NULL) {
        i = ((uintptr_t)f - (uintptr_t)str) / sizeof(STRING_char_t);
    }

    return i;
}

void STRF(erase, struct STRING * const s, const size_t idx, size_t len)
{
    const size_t size = STRF(size, s);

    if (idx >= size) {
        cstl_abort();
    }

    if (idx + len > size) {
        len = size - idx;
    }

    cstl_memmove(STRF(__at, s, idx),
                 STRF(__at, s, idx + len),
                 (size - (idx + len)) * sizeof(STRING_char_t));

    STRF(__resize, s, size - len);
}

void STRF(clear, struct STRING * const s)
{
    vector_clear(&s->v);
}

#undef STRING_char_t

#undef STDSTRF
#undef STDF
#undef STRV

#undef STRCAT
#undef _STRCAT

/*!
 * @} string
 */
