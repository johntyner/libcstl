/*!
 * @file
 */

#ifndef CSTL_STDLIB_H
#define CSTL_STDLIB_H

#include <sys/types.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <limits.h>

#define cstl_assert(x)          assert(x)

void cstl_abort(void);

int cstl_rand(void);

void * cstl_realloc(void *, size_t);
void * cstl_malloc(size_t);
void cstl_free(void *);

static inline void * cstl_memcpy(
    void * const d, const void * const s, const size_t n)
{
    return memcpy(d, s, n);
}

void * cstl_memmove(void *, const void *, size_t);
void * cstl_memset(void *, int, size_t);

int cstl_strcmp(const char *, const char *);
char * cstl_strchr(const char *, int);
char * cstl_strstr(const char *, const char *);

#endif
