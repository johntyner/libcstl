#ifndef CSTL_HASH_H
#define CSTL_HASH_H

#include <stddef.h>

#include "vector.h"
#include "slist.h"

struct hash_node
{
    unsigned long k;
    struct slist_node n;
};

struct hash
{
    struct vector v;

    size_t count;
    size_t off;
    size_t (* hash)(unsigned long, size_t);
};

size_t hash_div(unsigned long, size_t);
size_t hash_mul(unsigned long, size_t);

static inline void hash_init(struct hash * const h, const size_t off)
{
    VECTOR_INIT(&h->v, struct slist);
    h->count = 0;
    h->off = off;
    h->hash = NULL;
}
#define HASH_INIT(H, TYPE, HN)                  \
    hash_init(H, offsetof(TYPE, HN))

size_t hash_size(const struct hash * const h)
{
    return h->count;
}

void hash_resize(struct hash *,
                 size_t, size_t (* hash)(unsigned long, size_t));

void hash_insert(struct hash *, unsigned long, void *);
void * hash_search(struct hash *, unsigned long,
                   int (*)(const void *, void *), void *);
void hash_erase(struct hash *, void *);

int hash_walk(struct hash *, int (*)(void *, void *), void *);

void hash_swap(struct hash *, struct hash *);

void hash_clear(struct hash *, void (*)(void *));

#endif
