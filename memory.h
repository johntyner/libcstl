#ifndef CSTL_MEMORY_H
#define CSTL_MEMORY_H

#include "common.h"

#include <stddef.h>
#include <stdatomic.h>

typedef void (cstl_memory_free_t)(void *);

struct unique_ptr
{
    void * ptr;
};

static inline void unique_ptr_init(struct unique_ptr * const up)
{
    up->ptr = NULL;
}

void unique_ptr_alloc(struct unique_ptr *, size_t);

static inline void * unique_ptr_get(struct unique_ptr * const up)
{
    return up->ptr;
}

static inline void * unique_ptr_release(struct unique_ptr * const up)
{
    void * const p = up->ptr;
    up->ptr = NULL;
    return p;
}

cstl_memory_free_t * unique_ptr_get_free(struct unique_ptr *);
static inline void unique_ptr_swap(struct unique_ptr * const up1,
                                   struct unique_ptr * const up2)
{
    struct unique_ptr t;
    cstl_swap(up1, up2, &t, sizeof(t));
}

void unique_ptr_reset(struct unique_ptr *);

struct shared_ptr_data
{
    struct {
        atomic_size_t hard, soft;
        atomic_flag valid;
    } ref;
    struct unique_ptr up;
};

struct shared_ptr
{
    struct shared_ptr_data * data;
};

static inline void shared_ptr_init(struct shared_ptr * const sp)
{
    sp->data = NULL;
}

void shared_ptr_alloc(struct shared_ptr *, size_t);
void * shared_ptr_get(struct shared_ptr *);
void shared_ptr_share(struct shared_ptr *, struct shared_ptr *);
void shared_ptr_reset(struct shared_ptr *);

struct weak_ptr
{
    struct shared_ptr_data * data;
};

static inline void weak_ptr_init(struct weak_ptr * const wp)
{
    wp->data = NULL;
}

void weak_ptr_from(struct weak_ptr *, struct shared_ptr *);
void weak_ptr_hold(struct weak_ptr *, struct shared_ptr *);
void weak_ptr_reset(struct weak_ptr *);

#endif
