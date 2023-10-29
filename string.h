/*!
 * @file
 *
 * @see _string.h
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

#endif
