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
 * @defgroup string String
 * @ingroup highlevel
 * @brief Vector-like memory management of a collection of characters
 *
 * Unless otherwise noted, @p %cstl_STRING is a "templatized" parameter
 * and should be replaced with the type of string object in use, e.g.
 * @p cstl_string, @p cstl_wstring.
 *
 * For example, the equivalent of
 * @code
 * // templatized prototype
 * void cstl_STRING_init(struct cstl_STRING *);
 * @endcode
 * is
 * @code
 * // narrow string initialization
 * void cstl_string_init(struct cstl_string *);
 *
 * // wide string initialization
 * void cstl_wstring_init(struct cstl_wstring *);
 * @endcode
 */
/*!
 * @addtogroup string
 * @{
 */

#include "cstl/common.h"

#define STRV(NAME)              CSTL_TOKCAT(cstl_STRING, _##NAME)
#define STRF(NAME, ...)         STRV(NAME)(__VA_ARGS__)
#define STDSTRF(NAME, ...)      CSTL_TOKCAT(STDSTRPFX, NAME)(__VA_ARGS__)

#ifndef NO_DOC
#define cstl_STRING_char_t           STRV(char_t)
#endif

/*!
 * @brief Type of character in a string
 */
typedef STRCHAR STRV(char_t);

/*!
 * @brief A string object
 *
 * The string object holds a "string" of characters in a contiguous
 * area of memory followed by a nul character. The nul character is
 * always maintained by the object and not included in the size of
 * the string.
 */
struct cstl_STRING
{
    /*! @privatesection */
    struct cstl_vector v;
};

/*!
 * @brief Initialize a string object
 *
 * @param[in,out] s A pointer to the string object to initialize
 */
static inline void STRF(init, struct cstl_STRING * const s)
{
    cstl_vector_init(&s->v, sizeof(cstl_STRING_char_t));
}

/*!
 * @brief Get the number of characters in a string
 *
 * @param[in] s A pointer to a string object
 *
 * @return The number of characters (not including the
 *         object-maintained nul terminator) in the string
 */
static inline size_t STRF(size, const struct cstl_STRING * const s)
{
    size_t sz = cstl_vector_size(&s->v);
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
static inline size_t STRF(capacity, const struct cstl_STRING * const s)
{
    size_t cap = cstl_vector_capacity(&s->v);
    if (cap > 0) {
        cap--;
    }
    return cap;
}

/*!
 * @brief Request to increase the capacity of the string
 *
 * @param[in] s A pointer to the string object
 * @param[in] sz The number of characters the string should be able to hold
 *
 * Requests to decrease the capacity are ignored. Requests to increase
 * the capacity that fail do so quietly
 */
static inline void STRF(reserve, struct cstl_STRING * const s, const size_t sz)
{
    cstl_vector_reserve(&s->v, sz + 1);
}

/*!
 * @brief Change the number of valid characters in the string
 *
 * @param[in] s A pointer to the string
 * @param[in] sz The number of characters desired in the string
 *
 * The function attempts to set the number of valid characters to the
 * number indicated. If the number is less than or equal to the
 * current capacity of the string, the function always succeeds. If
 * the number exceeds the capacity, and the function cannot increase
 * the capacity, the function will cause an abort.
 *
 * During an increase, newly valid characters will be initialized to
 * the string's nul character
 */
void STRF(resize, struct cstl_STRING * s, size_t sz);

/*!
 * @brief Get a pointer to a character in the string
 *
 * @param[in] s A pointer to the string
 * @param[in] i The 0-based index of the desired character in the string
 *
 * @return A pointer to the character indicated by the index
 *
 * @note The code will cause an abort if the index is outside the range
 *       of valid character positions it the string
 */
cstl_STRING_char_t * STRF(at, struct cstl_STRING * s, size_t i);
/*!
 * @brief Get a const pointer to a character from a const string
 *
 * @param[in] s A pointer to the string
 * @param[in] i The 0-based index of the desired character in the string
 *
 * @return A pointer to the character indicated by the index
 *
 * @note The code will cause an abort if the index is outside the range
 *       of valid character positions it the string
 */
const cstl_STRING_char_t * STRF(at_const,
                                const struct cstl_STRING * s, size_t i);

/*!
 * @brief Get a pointer to the start of the string
 *
 * @param[in] s A pointer to the string object
 *
 * @note If the string is empty, the function may or may not return NULL
 *
 * @return A pointer to the start of the string data
 * @retval NULL The string is empty
 */
static inline cstl_STRING_char_t * STRF(data, struct cstl_STRING * const s)
{
    return cstl_vector_data(&s->v);
}

/*!
 * @brief Get a const pointer to the start of the string
 *
 * @param[in] s A pointer to the string object
 *
 * @note This function will always return a pointer to a valid string
 *
 * @return A const pointer to the start of the string data
 */
const cstl_STRING_char_t * STRF(str, const struct cstl_STRING * s);

/*!
 * @brief Compare a string object for (in)equality with a "raw" string
 *
 * The strings are compared as if by strlen(), and the result, as
 * per that function is returned.
 *
 * @param[in] s A pointer to a string object
 * @param[in] str A pointer to a "raw" string object
 *
 * @return An integer representing the result of the comparison as
 *         defined by strlen() or equivalent
 */
static inline int STRF(compare_str,
                       const struct cstl_STRING * const s,
                       const cstl_STRING_char_t * const str)
{
    return STDSTRF(cmp, STRF(str, s), str);
}
/*!
 * @brief Compare two string objects for (in)equality
 *
 * The strings are compared as if by strlen(), and the result, as
 * per that function is returned.
 *
 * @param[in] s1 A pointer to a string object
 * @param[in] s2 A pointer to a(nother) string object
 *
 * @return An integer representing the result of the comparison as
 *         defined by strlen() or equivalent
 */
static inline int STRF(compare,
                       const struct cstl_STRING * const s1,
                       const struct cstl_STRING * const s2)
{
    return STRF(compare_str, s1, STRF(str, s2));
}

/*!
 * @brief Return a string to its initialized state
 *
 * @param[in] s A pointer to a string object
 */
static inline void STRF(clear, struct cstl_STRING * const s)
{
    cstl_vector_clear(&s->v);
}

/*!
 * @brief Insert characters into a string object
 *
 * Insert @p cnt copies of the character @p ch into the string @p s
 * at the position denoted by @p pos. If @p pos > cstl_STRING_size(), the
 * function abort()s.
 *
 * @param[in] s A string into which to insert characters
 * @param[in] pos The position in @p s at which to insert the characters
 * @param[in] cnt The number of copies of @p ch to insert
 * @param[in] ch The character to insert into @p s
 */
void STRF(insert_ch,
          struct cstl_STRING * s, size_t pos, size_t cnt, cstl_STRING_char_t ch);
/*!
 * @brief Insert a string into a string object
 *
 * Insert the first @p n characters contained in @p str into the string @p s
 * at the position denoted by @p pos. If @p pos > cstl_STRING_size(), the
 * function abort()s.
 *
 * @param[in] s A string into which to insert characters
 * @param[in] pos The position in @p s at which to insert the characters
 * @param[in] str The string from which to draw characters to insert into @p s
 * @param[in] n The number of characters to draw from @p str
 */
void STRF(insert_str_n,
          struct cstl_STRING * s, size_t pos,
          const cstl_STRING_char_t * str, size_t n);
/*!
 * @brief Insert a string into a string object
 *
 * Insert the nul-terminated string pointed to @p str into the string @p s
 * at the position denoted by @p pos. If @p pos > cstl_STRING_size(), the
 * function abort()s.
 *
 * @param[in] s A string into which to insert characters
 * @param[in] pos The position in @p s at which to insert the characters
 * @param[in] str The string from which to draw characters to strert into @p s
 */
static inline void STRF(insert_str,
                        struct cstl_STRING * const s, const size_t pos,
                        const cstl_STRING_char_t * const str)
{
    STRF(insert_str_n, s, pos, str, STDSTRF(len, str));
}
/*!
 * @brief Insert a string into a string object
 *
 * Insert the characters contained in @p ins into the string @p s
 * at the position denoted by @p pos. If @p pos > cstl_STRING_size(), the
 * function abort()s.
 *
 * @param[in] s A string into which to insert characters
 * @param[in] pos The position in @p s at which to insert the characters
 * @param[in] ins The characters to insert into @p s
 */
static inline void STRF(insert,
                        struct cstl_STRING * s, size_t pos,
                        const struct cstl_STRING * ins)
{
    STRF(insert_str_n, s, pos, STRF(str, ins), STRF(size, ins));
}

/*!
 * @brief Append one string object to another
 *
 * Equivalent to @code
 * cstl_STRING_insert(s1, cstl_STRING_size(s1), s2)
 * @endcode
 *
 * @param[in] s1 The string to be extended
 * @param[in] s2 The string to be appended to @p s1
 */
static inline void STRF(append, struct cstl_STRING * const s1,
                        const struct cstl_STRING * const s2)
{
    STRF(insert, s1, STRF(size, s1), s2);
}

/*!
 * @brief Append a number of copies of a given character to a string
 *
 * Equivalent to @code
 * cstl_STRING_insert_ch(s, cstl_STRING_size(s), cnt, ch)
 * @endcode
 *
 * @param[in] s The string to be extended
 * @param[in] cnt The number of copies of @p ch to append
 * @param[in] ch The character to be appended
 */
static inline void STRF(append_ch, struct cstl_STRING * const s,
                        const size_t cnt, const cstl_STRING_char_t ch)
{
    STRF(insert_ch, s, STRF(size, s), cnt, ch);
}

/*!
 * @brief Append characters from a string to a string object
 *
 * Equivalent to @code
 * cstl_STRING_insert_str_n(s, cstl_STRING_size(s), str, len);
 * @endcode
 *
 * @param[in] s The string to be extended
 * @param[in] str The string from which to draw characters
 * @param[in] len The number of characters from @p str to append to @p s
 */
static inline void STRF(append_str_n, struct cstl_STRING * const s,
                        const cstl_STRING_char_t * const str,
                        const size_t len)
{
    STRF(insert_str_n, s, STRF(size, s), str, len);
}

/*!
 * @brief Append a string to a string object
 *
 * Equivalent to @code
 * cstl_STRING_insert_str(s, cstl_STRING_size(s), str)
 * @endcode
 *
 * @param[in] s The string to be extended
 * @param[in] str The string to be appended to @p s1
 */
static inline void STRF(append_str, struct cstl_STRING * const s,
                        const cstl_STRING_char_t * const str)
{
    STRF(insert_str, s, STRF(size, s), str);
}

/*!
 * @brief Set the contents of a string object to a "raw" string
 *
 * @param[in] s A pointer to a string object
 * @param[in] str A pointer to a nul-terminated string
 */
static inline void STRF(set_str, struct cstl_STRING * const s,
                        const cstl_STRING_char_t * const str)
{
    STRF(resize, s, 0);
    STRF(append_str, s, str);
}

/*!
 * @brief Remove contiguous characters from a string object
 *
 * Removes the @p n characters at @p pos from the string, @p n is
 * truncated, if necessary, to the number of characters between @p pos
 * and the end of the string.
 */
void STRF(erase, struct cstl_STRING * s, size_t pos, size_t n);

/*!
 * @brief Get a substring from a string object
 *
 * Returns an n-character string starting at a specified position. If the
 * substring indicated is longer than the number of characters possible, the
 * length of the substring will be truncated.
 *
 * @param[in] s A string from which to get the substring
 * @param[in] pos The starting position of the substring
 * @param[in] n The number of characters in the substring
 * @param[out] ss A pointer to an object containing the resulting substring
 */
void STRF(substr,
          const struct cstl_STRING * s, size_t pos, size_t n,
          struct cstl_STRING * ss);

/*!
 * @brief Find the first occurrence of a character in a string object
 *
 * Finds the first occurrence of the given character starting from
 * the given position in the string object. If the given position is
 * out of bounds, the function aborts.
 *
 * @param[in] s A pointer to the string object to be searched
 * @param[in] ch The character sought
 * @param[in] pos The position in the string at which to start the search
 */
ssize_t STRF(find_ch, const struct cstl_STRING * s, cstl_STRING_char_t ch,
             size_t pos);
/*!
 * @brief Find the first occurrence of a string in a string object
 *
 * Finds the first occurrence of the given string starting from
 * the given position in the string object. If the given position is
 * out of bounds, the function aborts.
 *
 * @param[in] hay A pointer to the string object to be searched
 * @param[in] ndl The string sought
 * @param[in] pos The position in the string at which to start the search
 */
ssize_t STRF(find_str,
             const struct cstl_STRING * hay,
             const cstl_STRING_char_t * ndl,
             size_t pos);
/*!
 * @brief Find the first occurrence of a string in a string object
 *
 * Finds the first occurrence of the given string starting from
 * the given position in the string object. If the given position is
 * out of bounds, the function aborts.
 *
 * @param[in] hay A pointer to the string object to be searched
 * @param[in] ndl The string sought
 * @param[in] pos The position in the string at which to start the search
 */
static inline ssize_t STRF(find, const struct cstl_STRING * const hay,
                           const struct cstl_STRING * const ndl,
                           const size_t pos)
{
    return STRF(find_str, hay, STRF(str, ndl), pos);
}

/*!
 * @brief Swap the string objects at the two given locations
 *
 * @param[in,out] s1 A pointer to a string
 * @param[in,out] s2 A pointer to a(nother) string
 *
 * The strings at the given locations will be swapped such that upon return,
 * @p a will contain the string previously pointed to by @p b and vice versa.
 */
static inline void STRF(
    swap, struct cstl_STRING * const s1, struct cstl_STRING * const s2)
{
    cstl_vector_swap(&s1->v, &s2->v);
}

/*!
 * @brief The nul character associated with a string type
 */
extern const cstl_STRING_char_t STRV(nul);

#undef cstl_STRING_char_t

#undef STDSTRF
#undef STRF
#undef STRV

/*!
 * @}
 */
