#include "stdlib.h"

#include <stdlib.h>

void cstl_abort(void)
{
    abort();
}

int cstl_rand(void)
{
    return rand();
}

void * cstl_realloc(void * const p, const size_t n)
{
    return realloc(p, n);
}

void * cstl_malloc(const size_t n)
{
    return malloc(n);
}

void cstl_free(void * const p)
{
    free(p);
}

void * cstl_memmove(void * const d, const void * const s, const size_t n)
{
    return memmove(d, s, n);
}

void * cstl_memset(void * const d, const int c, const size_t n)
{
    return memset(d, c, n);
}

int cstl_strcmp(const char * const s1, const char * const s2)
{
    return strcmp(s1, s2);
}

char * cstl_strchr(const char * const s, const int c)
{
    return strchr(s, c);
}

char * cstl_strstr(const char * const h, const char * const n)
{
    return strstr(h, n);
}
