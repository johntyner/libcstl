/*!
 * @file
 */

#ifndef CSTL_MEMORY_H
#define CSTL_MEMORY_H

#include "common.h"

typedef struct unique_ptr
{
    void * ptr;
    cstl_clear_func_t * clr;
} unique_ptr_t;

static inline void unique_ptr_init(struct unique_ptr * const up)
{
    up->ptr = NULL;
    up->clr = NULL;
}

void unique_ptr_alloc(struct unique_ptr *, size_t, cstl_clear_func_t *);

static inline void * unique_ptr_get(const struct unique_ptr * const up)
{
    return up->ptr;
}

static inline void * unique_ptr_release(struct unique_ptr * const up,
                                        cstl_clear_func_t ** const clr)
{
    void * const p = unique_ptr_get(up);
    if (clr != NULL) {
        *clr = up->clr;
    }
    unique_ptr_init(up);
    return p;
}

static inline void unique_ptr_swap(struct unique_ptr * const up1,
                                   struct unique_ptr * const up2)
{
    struct unique_ptr t;
    cstl_swap(up1, up2, &t, sizeof(t));
}

void unique_ptr_reset(struct unique_ptr *);

struct shared_ptr_data;
typedef struct shared_ptr
{
    struct shared_ptr_data * data;
} shared_ptr_t;

static inline void shared_ptr_init(struct shared_ptr * const sp)
{
    sp->data = NULL;
}

void shared_ptr_alloc(struct shared_ptr *, size_t, cstl_clear_func_t *);
void * shared_ptr_get(const struct shared_ptr *);
void shared_ptr_share(struct shared_ptr *, struct shared_ptr *);

static inline void shared_ptr_swap(struct shared_ptr * const sp1,
                                   struct shared_ptr * const sp2)
{
    struct shared_ptr t;
    cstl_swap(sp1, sp2, &t, sizeof(t));
}

void shared_ptr_reset(struct shared_ptr *);

typedef struct weak_ptr
{
    struct shared_ptr_data * data;
} weak_ptr_t;

static inline void weak_ptr_init(struct weak_ptr * const wp)
{
    wp->data = NULL;
}

void weak_ptr_from(struct weak_ptr *, struct shared_ptr *);
void weak_ptr_hold(struct weak_ptr *, struct shared_ptr *);

static inline void weak_ptr_swap(struct weak_ptr * const wp1,
                                 struct weak_ptr * const wp2)
{
    struct weak_ptr t;
    cstl_swap(wp1, wp2, &t, sizeof(t));
}

void weak_ptr_reset(struct weak_ptr *);

#endif
