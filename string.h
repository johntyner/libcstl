/*!
 * @file
 *
 * This is the public-facing header for string objects. The actual
 * implementation is in _string.[ch], hidden behind a lot of macros
 * that function as a poor man's template. This allows the same code
 * to implement both narrow and wide character string objects.
 *
 * This file sets the @p %STRING, @p %STRCHAR, and @p %STDSTRPFX macros,
 * and then includes the actual implementation file to import all of its
 * types, functions, and prototypes.
 *
 * To add new variants:
 *   - @p %STRING should be set to the name of the string
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

#include "vector.h"

#ifndef NO_DOC
#define STRING                  string
#define STRCHAR                 char
#define STDSTRPFX               str
#include "_string.h"
#undef STDSTRPFX
#undef STRCHAR
#undef STRING
#endif

#include <wchar.h>
#ifndef NO_DOC
#define STRING                  wstring
#define STRCHAR                 wchar_t
#define STDSTRPFX               wcs
#include "_string.h"
#undef STDSTRPFX
#undef STRCHAR
#undef STRING
#endif

/*!
 * @addtogroup string
 * @{
 */

/*!
 * @brief Constant initialization of a string object
 *
 * Unlike the templatized functions, "STRING" is not a placeholder
 * for a "templatized" name. This macro/function is called literally as
 * STRING_INITIALIZER() and the @p CTYPE parameter is the type of
 * character held by the string object, e.g. @p string_char_t,
 * @p wstring_char_t
 *
 * @param CTYPE The type of character that the string will hold
 */
#define STRING_INITIALIZER(CTYPE)               \
    {                                           \
        .v = VECTOR_INITIALIZER(CTYPE),         \
    }
/*!
 * @brief (Statically) declare and initialize a vector
 *
 * Unlike the templatized functions, "STRING" is not a placeholder
 * for a "templatized" name. This macro/function is called literally as
 * DECLARE_STRING() and the @p TYPE parameter is the type of string object
 * being declared, e.g. @p string, @p wstring.
 *
 * @param TYPE The type of string object being declared
 * @param NAME The name of the variable being declared
 */
#define DECLARE_STRING(TYPE, NAME)                      \
    struct TYPE NAME =                                  \
        STRING_INITIALIZER(TOKCAT(TYPE, _char_t))
/*!
 * @} string
 */

#endif
