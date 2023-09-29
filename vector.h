#ifndef CSTL_VECTOR_H
#define CSTL_VECTOR_H

#include <stddef.h>
#include <sys/types.h>

struct vector
{
    void * elem;
    size_t size;
    size_t count, cap;

    void (* cons)(void *);
    void (* dest)(void *);
};

static inline void vector_init(struct vector * const v,
                               const size_t sz,
                               void (* const cons)(void *),
                               void (* const dest)(void *))
{
    v->elem = NULL;
    v->size = sz;

    v->count = 0;
    v->cap   = 0;

    v->cons = cons;
    v->dest = dest;
}
#define VECTOR_INIT(V, TYPE, CONS, DEST)        \
    vector_init(V, sizeof(TYPE), CONS, DEST)

static inline size_t vector_size(const struct vector * const v)
{
    return v->count;
}

static inline size_t vector_capacity(const struct vector * const v)
{
    return v->cap;
}

/* will abort() if out of bounds */
void * vector_at(struct vector *, size_t);
const void * vector_at_const(const struct vector *, size_t);

/* this is a request and may (silently) fail */
void vector_set_capacity(struct vector *, size_t);
/* this is a request and may (silently) fail */
void vector_shrink_to_fit(struct vector *);

/* will return error on failure to increase size */
int __vector_resize(struct vector *, size_t);
/* will abort() on failure to increase size */
void vector_resize(struct vector *, size_t);

void vector_sort(struct vector *, int (*)(const void *, const void *));
ssize_t vector_search(const struct vector *,
                      const void *, int (*)(const void *, const void *));

void vector_reverse(struct vector *);

void vector_clear(struct vector *);

#endif
