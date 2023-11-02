/*!
 * @file
 */

#ifndef CSTL_MAP_H
#define CSTL_MAP_H

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
    void * val;

    /*! @private */
    void * _;
} cstl_map_iterator_t;

const cstl_map_iterator_t * cstl_map_iterator_end(const cstl_map_t *);
static inline bool cstl_map_iterator_eq(
    const cstl_map_iterator_t * const a, const cstl_map_iterator_t * const b)
{
    return a->_ == b->_;
}

void cstl_map_init(cstl_map_t *, cstl_compare_func_t *, void *);
int cstl_map_insert(cstl_map_t *, const void *, void *, cstl_map_iterator_t *);
void cstl_map_find(const cstl_map_t *, const void *, cstl_map_iterator_t *);
int cstl_map_erase(cstl_map_t *, const void *, const void **, void **);
void cstl_map_erase_iterator(cstl_map_t *, cstl_map_iterator_t *);
void cstl_map_clear(cstl_map_t *, cstl_xtor_func_t *, void *);

#endif
