/*!
 * @file
 */

#include "cstl/map.h"

#include <stdlib.h>

static const cstl_map_iterator_t CSTL_MAP_ITERATOR_END =
{
    ._ = NULL,

    .key = NULL,
    .val = NULL,
};

struct cstl_map_node
{
    cstl_unique_ptr_t key;
    cstl_shared_ptr_t val;

    struct cstl_rbtree_node n;
};

const cstl_map_iterator_t * cstl_map_iterator_end(const cstl_map_t * const m)
{
    return &CSTL_MAP_ITERATOR_END;
    (void)m;
}

bool cstl_map_iterator_eq(const cstl_map_iterator_t * const a,
                          const cstl_map_iterator_t * const b)
{
    return a->_ == b->_;
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

static void cstl_map_node_free(void * const p, void * const nil)
{
    struct cstl_map_node * const n = p;

    cstl_shared_ptr_reset(&n->val);
    cstl_unique_ptr_reset(&n->key);

    free(n);

    (void)nil;
}

static int cstl_map_node_cmp(const void * const _a, const void * const _b,
                             void * const p)
{
    cstl_map_t * const m = p;

    const struct cstl_map_node * const a = _a;
    const struct cstl_map_node * const b = _b;

    return m->cmp.f(cstl_unique_ptr_get(&a->key),
                    cstl_unique_ptr_get(&b->key), m->cmp.p);
}

static void __cstl_map_clear(void * const m, void * const nil)
{
    cstl_rbtree_clear(&((cstl_map_t *)m)->t, cstl_map_node_free);
    (void)nil;
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

cstl_map_t * cstl_map_new(cstl_compare_func_t * const cmp, void * const priv,
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
    return cstl_unique_ptr_get(m);
}

void cstl_map_find(const cstl_map_t * const map,
                   const void * const key,
                   cstl_map_iterator_t * const i)
{
    struct cstl_map_node node;

    cstl_unique_ptr_init_set(&node.key, (void *)key, NULL, NULL);
    i->_ = (void *)cstl_rbtree_find(&map->t, &node);
    cstl_unique_ptr_release(&node.key, NULL, NULL);

    if (i->_ != NULL) {
        cstl_map_iterator_init(i, i->_);
    } else {
        *i = *cstl_map_iterator_end(map);
    }
}

void cstl_map_erase(cstl_map_t * const map, const void * const key)
{
    cstl_map_iterator_t i;
    cstl_map_find(map, key, &i);
    if (i._ != NULL) {
        cstl_map_erase_iterator(map, &i);
    }
}

void cstl_map_erase_iterator(cstl_map_t * const map,
                             cstl_map_iterator_t * const i)
{
    struct cstl_map_node * const n = i->_;
    __cstl_rbtree_erase(&map->t, &n->n);
    cstl_map_node_free(n, NULL);
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
        *_i = i;
    }

    return err;
}

#ifdef __cfg_test__
#include <check.h>

#include "cstl/string.h"

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

static int map_key_cmp(const void * const a, const void * const b,
                       void * const nil)
{
    return cstl_string_compare((cstl_string_t *)a, (cstl_string_t *)b);
    (void)nil;
}

static void map_key_clr(void * const a, void * const nil)
{
    cstl_string_clear((cstl_string_t *)a);
    (void)nil;
}

/*
 * for the tests, the map is string->int with the strings
 * containing single letters "a" through "z" with their
 * corresponding integers being 0 through 25
 */
static int populate_map(cstl_map_t * const map)
{
    int i;

    /*
     * big assumption that 'a' is "numerically" less than 'z'
     * and that they're contiguous
     */
    for (i = (int)'a'; i < (int)'z'; i++) {
        DECLARE_CSTL_UNIQUE_PTR(k);
        DECLARE_CSTL_SHARED_PTR(v);

        cstl_string_t * str;
        int * integer;

        cstl_unique_ptr_alloc(&k, sizeof(*str), map_key_clr, NULL);
        str = cstl_unique_ptr_get(&k);
        cstl_string_init(str);
        cstl_string_resize(str, 1);
        *cstl_string_at(str, 0) = (char)i;

        cstl_shared_ptr_alloc(&v, sizeof(*integer), NULL);
        integer = cstl_shared_ptr_get(&v);
        *integer = i - (int)'a';

        cstl_map_insert(map, &k, &v, NULL);

        cstl_shared_ptr_reset(&v);
        cstl_unique_ptr_reset(&k);
    }

    return i - (int)'a';
}

START_TEST(fill)
{
    DECLARE_CSTL_UNIQUE_PTR(m);
    cstl_map_t * map;

    cstl_unique_ptr_init(&m);
    map = cstl_map_new(map_key_cmp, NULL, &m);
    populate_map(map);

    cstl_unique_ptr_reset(&m);
}
END_TEST

Suite * map_suite(void)
{
    Suite * const s = suite_create("map");

    TCase * tc;

    tc = tcase_create("map");
    tcase_add_test(tc, init);
    tcase_add_test(tc, fill);

    suite_add_tcase(s, tc);

    return s;
}

#endif
