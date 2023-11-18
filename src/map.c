/*!
 * @file
 */

#include "cstl/map.h"

#include <stdlib.h>

struct cstl_map_node
{
    const void * key;
    void * val;

    struct cstl_rbtree_node n;
};

const cstl_map_iterator_t * cstl_map_iterator_end(const cstl_map_t * const m)
{
    static const cstl_map_iterator_t end = {
        ._ = NULL,

        .key = NULL,
        .val = NULL,
    };

    return &end;
    (void)m;
}

/*! @private */
static void cstl_map_iterator_init(cstl_map_iterator_t * const i,
                                   struct cstl_map_node * const node)
{
    i->key = node->key;
    i->val = node->val;

    i->_ = node;
}


/*! @private */
static struct cstl_map_node * cstl_map_node_alloc(
    const void * const key, void * const val)
{
    struct cstl_map_node * const n = malloc(sizeof(*n));
    if (n) {
        n->key = key;
        n->val = val;
    }
    return n;
}

/*! @private */
static void cstl_map_node_free(void * const n)
{
    free(n);
}

/*! @private */
static int cstl_map_node_cmp(const void * const _a, const void * const _b,
                             void * const p)
{
    cstl_map_t * const m = p;

    const struct cstl_map_node * const a = _a;
    const struct cstl_map_node * const b = _b;

    return m->cmp.f(a->key, b->key, m->cmp.p);
}

/*! @private */
struct cmc_priv
{
    cstl_xtor_func_t * clr;
    void * priv;
};

/*! @private */
static void __cstl_map_node_clear(void * const n, void * const p)
{
    struct cmc_priv * const cmc = p;
    struct cstl_map_node * const node = n;

    if (cmc->clr != NULL) {
        cstl_map_iterator_t i;

        cstl_map_iterator_init(&i, node);
        i._ = NULL;

        cmc->clr(&i, cmc->priv);
    }

    cstl_map_node_free(node);
}

void cstl_map_clear(cstl_map_t * const map,
                    cstl_xtor_func_t * const clr, void * const priv)
{
    struct cmc_priv cmc;

    cmc.clr = clr;
    cmc.priv = priv;

    cstl_rbtree_clear(&((cstl_map_t *)map)->t, __cstl_map_node_clear, &cmc);
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

void cstl_map_find(const cstl_map_t * const map,
                   const void * const key,
                   cstl_map_iterator_t * const i)
{
    struct cstl_map_node node;

    node.key = key;
    i->_ = (void *)cstl_rbtree_find(&map->t, &node);

    if (i->_ != NULL) {
        cstl_map_iterator_init(i, i->_);
    } else {
        *i = *cstl_map_iterator_end(map);
    }
}

int cstl_map_erase(cstl_map_t * const map, const void * const key,
                   cstl_map_iterator_t * const _i)
{
    cstl_map_iterator_t i;
    int err;

    err = -1;
    cstl_map_find(map, key, &i);
    if (i._ != NULL) {
        cstl_map_erase_iterator(map, &i);
        err = 0;
    }

    if (_i != NULL) {
        *_i = i;
        _i->_ = NULL;
    }

    return err;
}

void cstl_map_erase_iterator(cstl_map_t * const map,
                             cstl_map_iterator_t * const i)
{
    struct cstl_map_node * const n = i->_;
    __cstl_rbtree_erase(&map->t, &n->n);
    cstl_map_node_free(n);
}

int cstl_map_insert(
    cstl_map_t * const map,
    const void * const key, void * const val,
    cstl_map_iterator_t * const _i)
{
    cstl_map_iterator_t i;
    int err;

    cstl_map_find(map, key, &i);
    if (cstl_map_iterator_eq(&i, cstl_map_iterator_end(map))) {
        /* no existing node in the map, carry on */
        struct cstl_map_node * const node = cstl_map_node_alloc(key, val);
        if (node != NULL) {
            cstl_rbtree_insert(&map->t, node);
            cstl_map_iterator_init(&i, node);
            err = 0;
        } else {
            err = -1;
        }
    } else {
        err = 1;
    }

    if (_i != NULL) {
        *_i = i;
    }

    return err;
}

#ifdef __cstl_cfg_test__
// GCOV_EXCL_START
#include <check.h>

#include "cstl/string.h"

START_TEST(init)
{
    cstl_map_t map;
    cstl_map_init(&map, NULL, NULL);
    cstl_map_clear(&map, NULL, NULL);
}

static int map_key_cmp(const void * const a, const void * const b,
                       void * const nil)
{
    return cstl_string_compare(
               (cstl_string_t *)a, (cstl_string_t *)b);

    (void)nil;
}

static void map_elem_clear(void * const e, void * const nil)
{
    cstl_map_iterator_t * const i = e;
    cstl_string_t * const s = (void *)i->key;

    free(i->val);
    cstl_string_clear(s);
    free(s);

    (void)nil;
}

/*
 * for the tests, the map is cstl_string->int with the chars
 * containing the letters 'a' through 'z' with their
 * corresponding integers being 0 through 25
 */
static void fill_map(cstl_map_t * const map)
{
    int j, res;

    /*
     * big assumption that 'a' is "numerically" less than 'z'
     * and that they're contiguous
     */
    for (j = 0; j < (int)('z' - 'a') + 1; j++) {
        cstl_map_iterator_t iter;
        cstl_string_t * s;
        int * i;

        s = malloc(sizeof(*s));
        ck_assert_ptr_ne(s, NULL);
        cstl_string_init(s);
        cstl_string_resize(s, 1);
        *cstl_string_data(s) = 'a' + j;

        i = malloc(sizeof(*i));
        ck_assert_ptr_nonnull(i);
        *i = j;

        res = cstl_map_insert(map, s, i, &iter);
        ck_assert_int_eq(res, 0);
        ck_assert_ptr_nonnull(iter._);
        ck_assert_ptr_eq(iter.key, s);
        ck_assert_ptr_eq(iter.val, i);
    }

    do {
        cstl_map_iterator_t iter;
        cstl_string_t * s;
        int * i;

        j = 12;

        s = malloc(sizeof(*s));
        ck_assert_ptr_ne(s, NULL);
        cstl_string_init(s);
        cstl_string_resize(s, 1);
        *cstl_string_data(s) = 'a' + j;

        i = malloc(sizeof(*i));
        ck_assert_ptr_nonnull(i);
        *i = j;

        /* insertion should fail because it's already in the map */
        res = cstl_map_insert(map, s, i, &iter);
        ck_assert_int_eq(res, 1);
        ck_assert_ptr_nonnull(iter._);
        ck_assert_ptr_ne(iter.key, s);
        ck_assert_ptr_ne(iter.val, i);

        free(i);
        cstl_string_clear(s);
        free(s);
    } while (0);
}

START_TEST(fill)
{
    cstl_map_t map;

    cstl_map_init(&map, map_key_cmp, NULL);
    fill_map(&map);
    cstl_map_clear(&map, map_elem_clear, NULL);
}
END_TEST

START_TEST(find)
{
    DECLARE_CSTL_STRING(string, s);
    cstl_map_t map;
    cstl_map_iterator_t i;

    cstl_map_init(&map, map_key_cmp, NULL);
    fill_map(&map);

    cstl_string_set_str(&s, "a");
    cstl_map_find(&map, &s, &i);
    ck_assert(!cstl_map_iterator_eq(&i, cstl_map_iterator_end(&map)));
    ck_assert_ptr_nonnull(i.key);
    ck_assert_ptr_nonnull(i.val);
    ck_assert_int_eq(*(int *)i.val, 0);

    cstl_string_set_str(&s, "j");
    cstl_map_find(&map, &s, &i);
    ck_assert(!cstl_map_iterator_eq(&i, cstl_map_iterator_end(&map)));
    ck_assert_ptr_nonnull(i.key);
    ck_assert_ptr_nonnull(i.val);
    ck_assert_int_eq(*(int *)i.val, 9);

    cstl_string_set_str(&s, "z");
    cstl_map_find(&map, &s, &i);
    ck_assert(!cstl_map_iterator_eq(&i, cstl_map_iterator_end(&map)));
    ck_assert_ptr_nonnull(i.key);
    ck_assert_ptr_nonnull(i.val);
    ck_assert_int_eq(*(int *)i.val, 25);

    cstl_map_clear(&map, map_elem_clear, NULL);
    cstl_string_clear(&s);
}
END_TEST

START_TEST(erase)
{
    DECLARE_CSTL_STRING(string, s);
    cstl_map_t map;
    cstl_map_iterator_t i;
    int res;

    cstl_map_init(&map, map_key_cmp, NULL);
    fill_map(&map);

    cstl_string_set_str(&s, "j");
    cstl_map_find(&map, &s, &i);
    ck_assert(!cstl_map_iterator_eq(&i, cstl_map_iterator_end(&map)));
    cstl_map_erase_iterator(&map, &i);
    ck_assert_ptr_nonnull(i.key);
    ck_assert_ptr_nonnull(i.val);
    map_elem_clear(&i, NULL);
    cstl_map_find(&map, &s, &i);
    ck_assert(cstl_map_iterator_eq(&i, cstl_map_iterator_end(&map)));

    cstl_string_set_str(&s, "m");
    res = cstl_map_erase(&map, &s, &i);
    ck_assert_int_eq(res, 0);
    ck_assert_ptr_nonnull(i.key);
    ck_assert_ptr_nonnull(i.val);
    map_elem_clear(&i, NULL);
    cstl_map_find(&map, &s, &i);
    ck_assert(cstl_map_iterator_eq(&i, cstl_map_iterator_end(&map)));

    cstl_map_clear(&map, map_elem_clear, NULL);
    cstl_string_clear(&s);
}

Suite * map_suite(void)
{
    Suite * const s = suite_create("map");

    TCase * tc;

    tc = tcase_create("map");
    tcase_add_test(tc, init);
    tcase_add_test(tc, fill);
    tcase_add_test(tc, find);
    tcase_add_test(tc, erase);

    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
