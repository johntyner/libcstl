#ifndef CSTL_VECTOR_H
#define CSTL_VECTOR_H

#include <stddef.h>
#include <sys/types.h>

struct vector
{
    struct {
        void * base;
        size_t size;
    } elem;
    size_t count, cap;
};

static inline void vector_construct(struct vector * const v, const size_t sz)
{
    v->elem.base = NULL;
    v->elem.size = sz;

    v->count = 0;
    v->cap   = 0;
}
#define VECTOR_CONSTRUCT(V, TYPE)    vector_construct(V, sizeof(TYPE))

static inline size_t vector_size(const struct vector * const v)
{
    return v->count;
}

static inline size_t vector_capacity(const struct vector * const v)
{
    return v->cap;
}

static inline void * vector_data(struct vector * const v)
{
    return v->elem.base;
}

/* will abort() if out of bounds */
void * vector_at(struct vector *, size_t);
const void * vector_at_const(const struct vector *, size_t);

/* this is a request and may (silently) fail */
void vector_reserve(struct vector *, size_t);
/* this is a request and may (silently) fail */
void vector_shrink_to_fit(struct vector *);

/* will return error on failure to increase size */
int __vector_resize(struct vector *, size_t);
/* will abort() on failure to increase size */
void vector_resize(struct vector *, size_t);

typedef enum {
    VECTOR_SORT_ALGORITHM_QUICK,
    VECTOR_SORT_ALGORITHM_QUICK_R,
    VECTOR_SORT_ALGORITHM_HEAP,

    VECTOR_SORT_ALGORITHM_DEFAULT = VECTOR_SORT_ALGORITHM_QUICK_R,
} vector_sort_algorithm_t;
void vector_sort(struct vector *,
                 int (*)(const void *, const void *),
                 vector_sort_algorithm_t);
ssize_t vector_search(const struct vector *,
                      const void *, int (*)(const void *, const void *));

void vector_reverse(struct vector *);

void vector_swap(struct vector *, struct vector *);

void vector_clear(struct vector *);

static inline void vector_destroy(struct vector * const v)
{
    vector_clear(v);
}

#endif
