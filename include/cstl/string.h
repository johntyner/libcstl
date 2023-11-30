/*!
 * @file
 *
 * This is the public-facing header for string objects. The actual
 * implementation is in _string.[ch], hidden behind a lot of macros
 * that function as a poor man's template. This allows the same code
 * to implement both narrow and wide character string objects.
 *
 * This file sets the @p %cstl_STRING, @p %STRCHAR, and @p %STDSTRPFX macros,
 * and then includes the actual implementation file to import all of its
 * types, functions, and prototypes.
 *
 * To add new variants:
 *   - @p %cstl_STRING should be set to the name of the string
 *     object, e.g. string, wstring
 *   - @p %STRCHAR should be the type of character housed by the string,
 *     e.g. char, wchar_t
 *   - @p %STDSTRPFX should be the prefix of standard library functions
 *     associated with the type of string being implemented. For example,
 *     the implementation relies on the @p strlen() function. For the narrow
 *     string, @p %STDSTRPFX is set to @p str to select @p strlen(). For
 *     the wide string, it would be @p wcs to select @p wcslen().
 *
 * @see _string.h for the templatized %string object header
 */

#ifndef CSTL_STRING_H
#define CSTL_STRING_H

#include "cstl/vector.h"

#ifndef NO_DOC
#define cstl_STRING             cstl_string
#define STRCHAR                 char
#define STDSTRPFX               str
#include "cstl/_string.h"
#undef STDSTRPFX
#undef STRCHAR
#undef cstl_STRING
#endif

#include <wchar.h>
#ifndef NO_DOC
#define cstl_STRING             cstl_wstring
#define STRCHAR                 wchar_t
#define STDSTRPFX               wcs
#include "cstl/_string.h"
#undef STDSTRPFX
#undef STRCHAR
#undef cstl_STRING
#endif

/*!
 * @addtogroup string
 * @{
 */

/*!
 * @name Supported string types
 * @{
 */
/*! @brief String of narrow characters */
typedef struct cstl_string cstl_string_t;
/*! @brief String of wide characters */
typedef struct cstl_wstring cstl_wstring_t;
/*!
 * @}
 */

/*!
 * @brief Constant initialization of a string object
 *
 * Unlike the templatized functions, "STRING" is not a placeholder
 * for a "templatized" name. This macro/function is called literally as
 * CSTL_STRING_INITIALIZER() and the @p CTYPE parameter is the type of
 * character held by the string object, e.g. @p cstl_string_char_t,
 * @p cstl_wstring_char_t
 *
 * @param CTYPE The type of character that the string will hold
 */
#define CSTL_STRING_INITIALIZER(CTYPE)          \
    {                                           \
        .v = CSTL_VECTOR_INITIALIZER(CTYPE),    \
    }
/*!
 * @brief (Statically) declare and initialize a vector
 *
 * Unlike the templatized functions, "STRING" is not a placeholder
 * for a "templatized" name. This macro/function is called literally as
 * DECLARE_CSTL_STRING() and the @p TYPE parameter is the type of string
 * object being declared without the @p cstl_ prefix, e.g.
 * @code{.c}
 * DECLARE_CSTL_STRING(string, s);
 * // or
 * DECLARE_CSTL_STRING(wstring, ws);
 * @endcode
 *
 * @param TYPE The type of string object being declared
 * @param NAME The name of the variable being declared
 */
#define DECLARE_CSTL_STRING(TYPE, NAME)                 \
    struct cstl_##TYPE NAME =                           \
        CSTL_STRING_INITIALIZER(cstl_##TYPE##_char_t)
/*!
 * @}
 */

#endif
