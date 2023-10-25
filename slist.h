/*!
 * @file
 */

#ifndef CSTL_SLIST_H
#define CSTL_SLIST_H

/*!
 * @defgroup slist Singly-linked list
 * @brief A linked list allowing traversal in the forward direction
 */
/*!
 * @addtogroup slist
 * @{
 */

#include "common.h"

struct slist_node
{
    /*! @privatesection */
    struct slist_node * n;
};

struct slist
{
    /*! @privatesection */
    struct slist_node h;

    size_t count;
    size_t off;
};

#define SLIST_INITIALIZER(TYPE, MEMB)           \
    {                                           \
        .h = {                                  \
            .n = NULL,                          \
        },                                      \
        .count = 0,                             \
        .off = offsetof(TYPE, MEMB),            \
    }
#define DECLARE_SLIST(NAME, TYPE, MEMB)                 \
    struct slist NAME = SLIST_INITIALIZER(TYPE, MEMB)

static inline void slist_init(struct slist * const sl, const size_t off)
{
    sl->h.n = NULL;
    sl->count = 0;
    sl->off = off;
}

static inline void slist_swap(struct slist * const a,
                              struct slist * const b)
{
    struct slist t;
    cstl_swap(a, b, &t, sizeof(t));
}

static inline size_t slist_size(const struct slist * sl)
{
    return sl->count;
}

void slist_insert_after(struct slist *, void *, void *);
void * slist_erase_after(struct slist *, void *);

void slist_push_front(struct slist *, void *);
void * slist_pop_front(struct slist *);

void * slist_front(const struct slist *);

void slist_reverse(struct slist *);
void slist_sort(struct slist *, cstl_compare_func_t *, void *);
void slist_concat(struct slist *, struct slist *);

int slist_foreach(struct slist *, cstl_visit_func_t *, void *);
void slist_clear(struct slist *, cstl_clear_func_t *);

/*! @} slist */

#endif
