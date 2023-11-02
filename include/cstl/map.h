/*!
 * @file
 */

#ifndef CSTL_MAP_H
#define CSTL_MAP_H

#include "cstl/memory.h"
#include "cstl/rbtree.h"

#include <stdbool.h>

struct cstl_map
{
    struct cstl_rbtree t;
    struct {
        cstl_compare_func_t * f;
        void * p;
    } cmp;
};

typedef struct cstl_map cstl_map_t;

typedef struct
{
    const void * key;
    const cstl_shared_ptr_t * val;

    void * _;
} cstl_map_iterator_t;

bool cstl_map_iterator_eq(const cstl_map_iterator_t *,
                          const cstl_map_iterator_t *);
void cstl_map_iterator_end(cstl_map_iterator_t *, const cstl_map_t *);

void cstl_map_new(cstl_compare_func_t *, void *, cstl_unique_ptr_t *);
void cstl_map_init(cstl_map_t *, cstl_compare_func_t *, void *);
int cstl_map_insert(cstl_map_t *,
                    cstl_unique_ptr_t *, cstl_shared_ptr_t *,
                    cstl_map_iterator_t *);
void cstl_map_find(const cstl_map_t *, const void *, cstl_map_iterator_t *);
void cstl_map_erase(cstl_map_t *, const void *);
void cstl_map_erase_i(cstl_map_t *, cstl_map_iterator_t *);
void cstl_map_clear(cstl_map_t *);

#endif
