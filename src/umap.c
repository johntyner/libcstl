#include "cstl/umap.h"
#include "cstl/crc.h"

#include <stdlib.h>

unsigned long cstl_umap_keyhash(const void * obj, size_t len)
{
#if __SIZEOF_LONG__ == 4
    return cstl_crc32le(NULL, 0x04c11db7, ~0, obj, len) ^ ~0;
#elif __SIZEOF_LONG__ == 8
    return cstl_crc64le(NULL, 0x1b, ~0, obj, len) ^ ~0;
#else
#error "cstl_umap_keyhash is not implemented"
#endif
}

struct cstl_umap_node
{
    const void * key;
    void * val;

    struct cstl_hash_node hn;
};

static void cstl_umap_node_free(struct cstl_umap_node * node)
{
    free(node);
}

void cstl_umap_iterator_init(cstl_umap_t * const map,
                             cstl_umap_iterator_t * const i,
                             struct cstl_umap_node * const node)
{
    (void)map;

    i->key = node->key;
    i->val = node->val;

    i->_ = node;
}

// TODO: init functions have been returning void up til now.
// in this particular case, if hash is NULL, the table isn't
// properly initialized. need to determine if this function
// will return an error in that case or if it can be handled
// some other way
void cstl_umap_init(cstl_umap_t * const map,
                    cstl_umap_hash_func_t * const hash,
                    float maxlf, size_t rsrvd)
{
    cstl_hash_init(&map->htab, offsetof(struct cstl_umap_node, hn));
    map->hash = hash;

    if (maxlf <= 0.0f) {
        maxlf = 0.8f;
    }
    map->maxlf = maxlf;

    if (rsrvd <= 0) {
        rsrvd = 32;
    }
    cstl_umap_reserve(map, rsrvd);
}

void cstl_umap_reserve(cstl_umap_t * const map, const size_t rsv)
{
    const size_t nb = (float)rsv / map->maxlf;

    if (nb > cstl_hash_bucket_count(&map->htab)) {
        cstl_hash_resize(&map->htab, nb, NULL);
    }
}

struct cumc_priv
{
    cstl_umap_t * umap;
    cstl_xtor_func_t * dtor;
    void * priv;
};

static void __cstl_umap_node_clear(void * const e, void * const p)
{
    struct cstl_umap_node * const node = e;
    struct cumc_priv * const cumc = p;

    if (cumc->dtor != NULL) {
        cstl_umap_iterator_t i;

        cstl_umap_iterator_init(cumc->umap, &i, node);
        i._ = NULL;

        cumc->dtor(&i, cumc->priv);
    }

    cstl_umap_node_free(node);
}

void cstl_umap_clear(cstl_umap_t * const map,
                     cstl_xtor_func_t * const dtor, void * const priv)
{
    struct cumc_priv cumc;

    cumc.umap = map;
    cumc.dtor = dtor;
    cumc.priv = priv;

    cstl_hash_clear(&map->htab, __cstl_umap_node_clear, &cumc);
}

#ifdef __cfg_test__
// GCOV_EXCL_START
#include <check.h>

START_TEST(init)
{
    cstl_umap_t map;
    cstl_umap_init(&map, NULL, 0.0f, 0);
    cstl_umap_clear(&map, NULL, NULL);
}

Suite * umap_suite(void)
{
    Suite * const s = suite_create("umap");

    TCase * tc;

    tc = tcase_create("umap");
    tcase_add_test(tc, init);
    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
