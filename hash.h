/*!
 * @file
 */

#ifndef CSTL_HASH_H
#define CSTL_HASH_H

#include "common.h"

struct hash_node
{
    unsigned long k;
    struct hash_node * n;
};

struct hash
{
    struct {
        struct hash_node ** v;
        size_t n;
    } b;

    size_t count;
    size_t off;

    size_t (* hash)(unsigned long, size_t);
};

size_t hash_div(unsigned long, size_t);
size_t hash_mul(unsigned long, size_t);

static inline void hash_init(struct hash * const h, const size_t off)
{
    h->b.v = NULL;
    h->b.n = 0;

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
                   cstl_const_visit_func_t *, void *);
void hash_erase(struct hash *, void *);

int hash_foreach(struct hash *, cstl_visit_func_t *, void *);

static inline void hash_swap(struct hash * const h1, struct hash * const h2)
{
    struct hash t;
    cstl_swap(h1, h2, &t, sizeof(t));
}

void hash_clear(struct hash *, cstl_clear_func_t *);

#endif
