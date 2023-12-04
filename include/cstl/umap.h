#ifndef CSTL_UMAP_H
#define CSTL_UMAP_H

#include "cstl/common.h"
#include "cstl/hash.h"

#include <stddef.h>

typedef unsigned long cstl_umap_hash_func_t(const void *);

typedef struct
{
    struct cstl_hash htab;

    cstl_umap_hash_func_t * hash;
    float maxlf;
} cstl_umap_t;

/*!
 * @brief A pointer to an element within the map
 */
typedef struct
{
    /*! @brief Pointer to the key associated with the element */
    const void * key;
    /*! @brief Pointer to the value contained by the element */
    void * val;

    /*! @private */
    void * _;
} cstl_umap_iterator_t;

void cstl_umap_init(cstl_umap_t * map,
                    cstl_umap_hash_func_t * hash,
                    const float maxlf, const size_t rsrvd);

void cstl_umap_reserve(cstl_umap_t * map, size_t n);

void cstl_umap_clear(cstl_umap_t * map,
                     cstl_xtor_func_t * dtor, void * priv);

unsigned long cstl_umap_keyhash(const void * obj, size_t len);

#endif
