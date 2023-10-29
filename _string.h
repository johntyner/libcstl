/*!
 * @file
 */

/*
 * no include guard. this file is meant to be a template
 * that can be included multiple times. Prior to including
 * this file, the following macros must be defined:
 *  - TODO
 */

/*!
 * @defgroup string Strings
 * @brief Vector-like memory management of strings
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

/*!
 * @brief Type of character in a string
 */
typedef STRCHAR STRV(char_t);

struct STRING
{
    /*! @privatesection */
    struct vector v;
};

extern const STRING_char_t STRV(nul);

/*!
 * @brief Initialize a string object
 *
 * @param[in,out] s A pointer to the string object to initialize
 */
static inline void STRF(init, struct STRING * const s)
{
    vector_init(&s->v, sizeof(STRING_char_t), NULL, 0);
}

static inline size_t STRF(size, const struct STRING * const s)
{
    size_t sz = vector_size(&s->v);
    if (sz > 0) {
        sz--;
    }
    return sz;
}

/*!
 * @brief Get the number of characters a string can hold
 *
 * @param[in] s A pointer to a string object
 *
 * @return The number of characters the string object can hold
 */
static inline size_t STRF(capacity, const struct STRING * const s)
{
    size_t cap = vector_capacity(&s->v);
    if (cap > 0) {
        cap--;
    }
    return cap;
}

static inline void STRF(reserve, struct STRING * const s, const size_t n)
{
    vector_reserve(&s->v, n + 1);
}

void STRF(resize, struct STRING *, size_t);

STRING_char_t * STRF(at, struct STRING *, size_t);
const STRING_char_t * STRF(at_const, const struct STRING *, size_t);

STRING_char_t * STRF(data, struct STRING * const s)
{
    return vector_data(&s->v);
}

const STRING_char_t * STRF(str, const struct STRING *);

int STRF(compare, const struct STRING *, const struct STRING *);
int STRF(compare_str, const struct STRING *, const STRING_char_t *);

void STRF(clear, struct STRING *);

void STRF(insert, struct STRING *, size_t, const struct STRING *);
void STRF(insert_ch, struct STRING *, size_t, size_t, STRING_char_t);
void STRF(insert_str_n, struct STRING *, size_t,
          const STRING_char_t *, size_t);
static inline void STRF(insert_str, struct STRING * const s, const size_t i,
                        const STRING_char_t * const str)
{
    STRF(insert_str_n, s, i, str, STDSTRF(len, str));
}

static inline void STRF(append, struct STRING * const s1,
                        const struct STRING * const s2)
{
    STRF(insert, s1, STRF(size, s1), s2);
}

static inline void STRF(append_ch, struct STRING * const s,
                        const size_t cnt, const STRING_char_t ch)
{
    STRF(insert_ch, s, STRF(size, s), cnt, ch);
}

static inline void STRF(append_str_n, struct STRING * const s,
                        const STRING_char_t * const str,
                        const size_t len)
{
    STRF(insert_str_n, s, STRF(size, s), str, len);
}

static inline void STRF(append_str, struct STRING * const s,
                        const STRING_char_t * const str)
{
    STRF(insert_str, s, STRF(size, s), str);
}

static inline void STRF(set_str, struct STRING * const s,
                        const STRING_char_t * const str)
{
    STRF(clear, s);
    STRF(append_str, s, str);
}

void STRF(erase, struct STRING *, size_t, size_t);

void STRF(substr, const struct STRING *, size_t, size_t, struct STRING *);

ssize_t STRF(find_ch, const struct STRING *, STRING_char_t, size_t);
ssize_t STRF(find_str, const struct STRING *, const STRING_char_t *, size_t);
static inline ssize_t STRF(find, const struct STRING * const h,
                           const struct STRING * const n,
                           const size_t p)
{
    return STRF(find_str, h, STRF(str, n), p);
}

void STRF(swap, struct STRING * const s1, struct STRING * const s2)
{
    vector_swap(&s1->v, &s2->v);
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
