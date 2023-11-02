/*!
 * @file
 */

#include "cstl/map.h"

#include <stdlib.h>

struct cstl_map_node
{
    cstl_unique_ptr_t key;
    cstl_shared_ptr_t val;

    struct cstl_rbtree_node n;
};

void cstl_map_iterator_end(cstl_map_iterator_t * const i,
                           const cstl_map_t * const m)
{
    (void)m;

    i->key = NULL;
    i->val = NULL;

    i->_ = NULL;
}
void cstl_map_iterator_init(cstl_map_iterator_t * const i,
                            struct cstl_map_node * const node)
{
    i->key = cstl_unique_ptr_get(&node->key);
    i->val = &node->val;

    i->_ = node;
}


static struct cstl_map_node * cstl_map_node_alloc(void)
{
    struct cstl_map_node * const n = malloc(sizeof(*n));
    if (n) {
        cstl_unique_ptr_init(&n->key);
        cstl_shared_ptr_init(&n->val);
    }
    return n;
}

static void cstl_map_node_free(void * const p, void * const x)
{
    struct cstl_map_node * const n = p;

    (void)x;

    cstl_unique_ptr_reset(&n->key);
    cstl_shared_ptr_reset(&n->val);

    free(n);
}

static int cstl_map_node_cmp(const void * const _a,
                             const void * const _b,
                             void * const p)
{
    cstl_map_t * const m = p;

    const struct cstl_map_node * const a = _a;
    const struct cstl_map_node * const b = _b;

    return m->cmp.f(cstl_unique_ptr_get(&a->key),
                    cstl_unique_ptr_get(&b->key), m->cmp.p);
}

static void __cstl_map_clear(void * const m, void * const p)
{
    (void)p;
    cstl_rbtree_clear(&((cstl_map_t *)m)->t, cstl_map_node_free);
}

void cstl_map_clear(cstl_map_t * const map)
{
    __cstl_map_clear(map, NULL);
}

void cstl_map_init(cstl_map_t * const map,
                   cstl_compare_func_t * const cmp, void * const priv)
{
    map->cmp.f = cmp;
    map->cmp.p = priv;

    cstl_rbtree_init(&map->t,
                     cstl_map_node_cmp, map,
                     offsetof(struct cstl_map_node, n));
}

void cstl_map_new(cstl_compare_func_t * const cmp, void * const priv,
                  cstl_unique_ptr_t * const m)
{
    cstl_unique_ptr_t map;

    cstl_unique_ptr_init(&map);
    cstl_unique_ptr_alloc(&map, sizeof(cstl_map_t), __cstl_map_clear, NULL);

    if (cstl_unique_ptr_get(&map) != NULL) {
        cstl_map_init(cstl_unique_ptr_get(&map), cmp, priv);
        cstl_unique_ptr_swap(m, &map);
    }

    cstl_unique_ptr_reset(&map);
}

void cstl_map_find(const cstl_map_t * const map,
                   const void * const key,
                   cstl_map_iterator_t * const i)
{
    struct cstl_map_node cmp_node;
    const struct cstl_map_node * f;

    cstl_map_iterator_end(i, map);

    cstl_unique_ptr_init_set(&cmp_node.key, (void *)key, NULL, NULL);
    f = i->_ = (void *)cstl_rbtree_find(&map->t, &cmp_node);
    cstl_unique_ptr_release(&cmp_node.key, NULL, NULL);

    if (f != NULL) {
        i->key = cstl_unique_ptr_get(&f->key);
        i->val = &f->val;
    }
}

void cstl_map_erase(cstl_map_t * const map, const void * const key)
{
    cstl_map_iterator_t i;
}

int cstl_map_insert(
    cstl_map_t * const map,
    cstl_unique_ptr_t * const key, cstl_shared_ptr_t * const val,
    cstl_map_iterator_t * const _i)
{
    cstl_map_iterator_t i;
    int err;

    err = -1;
    cstl_map_find(map, cstl_unique_ptr_get(key), &i);
    if (i._ == NULL) {
        /* no existing node in the map, carry on */
        struct cstl_map_node * const node = cstl_map_node_alloc();
        if (node != NULL) {
            cstl_unique_ptr_swap(key, &node->key);
            cstl_shared_ptr_swap(val, &node->val);

            cstl_rbtree_insert(&map->t, node);

            cstl_map_iterator_init(&i, node);
            err = 0;
        }
    }

    if (_i != NULL) {
        memcpy(_i, &i, sizeof(*_i));
    }

    return err;
}

#ifdef __cfg_test__
#include <check.h>

START_TEST(init)
{
    DECLARE_CSTL_UNIQUE_PTR(m);
    cstl_map_t map;

    cstl_map_init(&map, NULL, NULL);
    cstl_map_clear(&map);

    cstl_unique_ptr_init(&m);
    cstl_map_new(NULL, NULL, &m);
    cstl_unique_ptr_reset(&m);
}

Suite * map_suite(void)
{
    Suite * const s = suite_create("map");

    TCase * tc;

    tc = tcase_create("map");
    tcase_add_test(tc, init);

    suite_add_tcase(s, tc);

    return s;
}

#endif
