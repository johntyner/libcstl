#ifndef CSTL_LIST_H
#define CSTL_LIST_H

#include <stddef.h>

struct list_node
{
    struct list_node * p, * n;
};

struct list
{
    struct list_node h;
    size_t size;

    size_t off;
};

static inline void list_init(struct list * const l, const size_t off)
{
    l->h.p  = &l->h;
    l->h.n  = &l->h;

    l->size = 0;

    l->off  = off;
}
#define LIST_INIT(L, TYPE, MEMB)                \
    list_init(L, offsetof(TYPE, MEMB))

static inline size_t list_size(const struct list * const l)
{
    return l->size;
}

void list_insert(struct list *, void *, void *);
void list_erase(struct list *, void *);

void * list_front(struct list *);
void * list_back(struct list *);

void list_push_front(struct list *, void *);
void list_push_back(struct list *, void *);

void * list_pop_front(struct list *);
void * list_pop_back(struct list *);

void list_reverse(struct list *);
void list_sort(struct list *, int (*)(const void *, const void *));

void list_concat(struct list *, struct list *);

typedef enum {
    LIST_WALK_DIR_FWD,
    LIST_WALK_DIR_REV,
} list_foreach_dir_t;

int list_foreach(struct list *,
                 int (*)(void *, void *), void *,
                 list_foreach_dir_t);
void * list_find(const struct list *,
                 const void *, int (*)(const void *, const void *),
                 list_foreach_dir_t);

void list_swap(struct list *, struct list *);

void list_clear(struct list *, void (*)(void *));

#endif
