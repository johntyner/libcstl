/*!
 * @file
 */

#include "cstl/hash.h"

#include <stdlib.h>
#include <math.h>

size_t cstl_hash_div(const size_t k, const size_t m)
{
    return k % m;
}

size_t cstl_hash_mul(const size_t k, const size_t m)
{
    static const float phi = 1.61803398875f;
    const float M = phi * k;
    return (M - floorf(M)) * m;
}

/*!
 * @private
 *
 * Given a cstl_hash_node, return the containing object
 */
static void * __cstl_hash_element(const struct cstl_hash * const h,
                                  const struct cstl_hash_node * const n)
{
    return (void *)((uintptr_t)n - h->off);
}

/*!
 * @private
 *
 * Given an object, return the contained cstl_hash_node
 */
static struct cstl_hash_node * __cstl_hash_node(
    const struct cstl_hash * const h, const void * const e)
{
    return (void *)((uintptr_t)e + h->off);
}

/*! @private */
static struct cstl_hash_bucket * __cstl_hash_get_bucket(
    struct cstl_hash * const h, const size_t k,
    cstl_hash_func_t * const hash, const size_t count)
{
    const size_t i = hash(k, count);
    if (i >= count) {
        abort();
    }
    return &h->bucket.at[i];
}

#ifndef NO_DOC
#define HASH_LIST_FOREACH(HEAD, CURR, NEXT)                             \
    NEXT = HEAD; while ((CURR = NEXT) != NULL && (NEXT = (CURR)->next, !0))
#define HASH_LIST_INSERT(HEAD, N)               \
    (N)->next = HEAD; HEAD = N
#endif

/*! @private */
static void cstl_clean_bucket(
    struct cstl_hash * const h, struct cstl_hash_bucket * const bk)
{
    /* only clean dirty buckets */
    if (h->bucket.cst != bk->cst) {
        struct cstl_hash_node * n, * nn;

        /*
         * move the list of nodes to a temporary location.
         * this prevents any confusion if the cleaning results
         * in a node being hashed back into the same bucket
         */
        n = bk->n;
        bk->n = NULL;

        /*
         * for each node in the list, find it's new bucket
         * and put the node into it. no need to remove from
         * the current bucket since it's a singly linked list
         */
        HASH_LIST_FOREACH(n, n, nn) {
            struct cstl_hash_bucket * const _bk =
                __cstl_hash_get_bucket(
                    h, n->key, h->bucket.rh.hash, h->bucket.rh.count);
            HASH_LIST_INSERT(_bk->n, n);
        }

        /* the bucket is clean, now */
        bk->cst = h->bucket.cst;
    }
}

/*! @private */
static void __cstl_hash_rehash(struct cstl_hash * const h, size_t n)
{
    /* skip over already-cleaned buckets */
    while (h->bucket.rh.clean < h->bucket.count
           && h->bucket.at[h->bucket.rh.clean].cst == h->bucket.cst) {
        h->bucket.rh.clean++;
    }

    while (h->bucket.rh.clean < h->bucket.count && n > 0) {
        cstl_clean_bucket(h, &h->bucket.at[h->bucket.rh.clean]);
        h->bucket.rh.clean++;
        n--;
    }

    if (h->bucket.rh.clean >= h->bucket.count) {
        /* everything is clean; mark the rehash as complete */
        h->bucket.count = h->bucket.rh.count;
        h->bucket.hash = h->bucket.rh.hash;

        h->bucket.rh.hash = NULL;
    }
}

void cstl_hash_rehash(struct cstl_hash * const h)
{
    if (h->bucket.rh.hash != NULL) {
        __cstl_hash_rehash(h, SIZE_MAX);
    }
}

/*!
 * @private
 *
 * Given a key, return the associated hash bucket
 */
static struct cstl_hash_bucket * cstl_hash_get_bucket(
    struct cstl_hash * const h, const size_t k)
{
    struct cstl_hash_bucket * bk;

    bk = __cstl_hash_get_bucket(h, k, h->bucket.hash, h->bucket.count);

    /*
     * if rh.hash != NULL, then a rehash (following a resize or
     * changing of the hash function) is in progress. try to
     * clean a few buckets to move the process along.
     */
    if (h->bucket.rh.hash != NULL) {
        struct cstl_hash_bucket * const _bk =
            __cstl_hash_get_bucket(
                h, k, h->bucket.rh.hash, h->bucket.rh.count);

        /*
         * for the given key, clean the bucket at the old
         * location and at the new one. this ensures that
         * regularly used buckets are cleaned quickly.
         */
        cstl_clean_bucket(h, bk);
        cstl_clean_bucket(h, _bk);

        /*
         * also clean one more bucket. this ensures that
         * infrequently used buckets get cleaned and the
         * rehash completes in a reasonable amount of time
         */
        __cstl_hash_rehash(h, 1);

        bk = _bk;
    }

    return bk;
}

/*!
 * @private
 *
 * Visit each element within a particular bucket in the hash table
 */
static int cstl_hash_bucket_foreach(
    const struct cstl_hash * const h, struct cstl_hash_node * n,
    cstl_visit_func_t * const visit, void * const p)
{
    struct cstl_hash_node * nn;
    int res = 0;

    HASH_LIST_FOREACH(n, n, nn) {
        if ((res = visit(__cstl_hash_element(h, n), p)) != 0) {
            break;
        }
    }

    return res;
}

/*! @private */
static int __cstl_hash_foreach(const struct cstl_hash * const h,
                               cstl_visit_func_t * const visit, void * const p)
{
    int res;
    unsigned int i;

    for (i = 0, res = 0; i < h->bucket.count && res == 0; i++) {
        res = cstl_hash_bucket_foreach(h, h->bucket.at[i].n, visit, p);
    }

    return res;
}

int cstl_hash_foreach(struct cstl_hash * const h,
                      cstl_visit_func_t * const visit, void * const p)
{
    cstl_hash_rehash(h);
    return __cstl_hash_foreach(h, visit, p);
}

struct cstl_hash_foreach_visit_priv
{
    cstl_const_visit_func_t * visit;
    void * priv;
};

/*! @private */
static int cstl_hash_foreach_visit(void * const e, void * const p)
{
    struct cstl_hash_foreach_visit_priv * const hfvp = p;
    return hfvp->visit(e, hfvp->priv);
}

int cstl_hash_foreach_const(const struct cstl_hash * const h,
                            cstl_const_visit_func_t * const visit,
                            void * const p)
{
    /*
     * the caller's function will be given a const pointer to
     * the objects in the table, but internally, the foreach
     * function generates a callback with a non-const pointer.
     * we use an intermediate function that receives the non-const
     * pointer but then calls the user's function which receives
     * a const pointer.
     */
    struct cstl_hash_foreach_visit_priv hfvp;
    hfvp.visit = visit;
    hfvp.priv = p;
    return __cstl_hash_foreach(h, cstl_hash_foreach_visit, &hfvp);
}

/*! @private */
static void __cstl_hash_set_capacity(
    struct cstl_hash * const h, const size_t sz)
{
    struct cstl_hash_bucket * const at =
        realloc(h->bucket.at, sizeof(*at) * sz);
    if (at != NULL) {
        h->bucket.at = at;
        h->bucket.capacity = sz;
    }
}

void cstl_hash_resize(struct cstl_hash * const h,
                      const size_t count, cstl_hash_func_t * const hash)
{
    if (count > 0) {
        if (count > h->bucket.capacity) {
            __cstl_hash_set_capacity(h, count);
        }

        if (h->bucket.at != NULL
            && count <= h->bucket.capacity
            && (count != h->bucket.count
                || (hash != NULL
                    && hash != h->bucket.hash))) {
            unsigned int i;

            /*
             * can't resize until the previous one
             * has finished. force it to finish now.
             */
            cstl_hash_rehash(h);

            h->bucket.cst = !h->bucket.cst;

            /*
             * buckets successfully allocated. initialize them.
             */
            for (i = h->bucket.count; i < count; i++) {
                h->bucket.at[i].n = NULL;
                /* newly added buckets are clean */
                h->bucket.at[i].cst = h->bucket.cst;
            }

            /*
             * set the new hash function. there is no requirement
             * that it be the same function as the current one
             */
            if (hash != NULL) {
                h->bucket.rh.hash = hash;
            } else if (h->bucket.hash != NULL) {
                h->bucket.rh.hash = h->bucket.hash;
            } else {
                h->bucket.rh.hash = cstl_hash_mul;
            }
            h->bucket.rh.count = count;
            h->bucket.rh.clean = 0;

            if (h->bucket.hash == NULL) {
                /* first resize */
                h->bucket.hash = h->bucket.rh.hash;
                h->bucket.count = h->bucket.rh.count;

                h->bucket.rh.hash = NULL;
            }
        } /*
           * else, allocation of buckets failed.
           * nothing to clean up; just exit
           */
    }
}

void cstl_hash_shrink_to_fit(struct cstl_hash * const h)
{
    size_t count;

    count = h->bucket.count;
    if (h->bucket.rh.hash != NULL) {
        count = h->bucket.rh.count;
    }

    if (h->bucket.capacity > count) {
        cstl_hash_rehash(h);
        __cstl_hash_set_capacity(h, h->bucket.count);
    }
}

void cstl_hash_insert(struct cstl_hash * const h,
                      const size_t k, void * const e)
{
    struct cstl_hash_bucket * const bk = cstl_hash_get_bucket(h, k);
    struct cstl_hash_node * const hn = __cstl_hash_node(h, e);

    hn->key = k;

    /*
     * the bucket is a singly-linked/forward list.
     * insert the new object at the front of the list.
     */
    HASH_LIST_INSERT(bk->n, hn);

    h->count++;
}

/*! @private */
struct cstl_hash_find_priv
{
    /* the hash, obviously */
    struct cstl_hash * h;
    /* the key being sought */
    size_t k;
    cstl_const_visit_func_t * visit;
    void * p;
    /*
     * initially null, set to the found object
     * if the visit function indicates that it
     * is the object being sought
     */
    const void * e;
};

/*! @private */
static int cstl_hash_find_visit(void * const e, void * const p)
{
    struct cstl_hash_find_priv * const hfp = p;

    /* only visit nodes with matching keys */
    if (__cstl_hash_node(hfp->h, e)->key == hfp->k) {
        /*
         * if there is no visit function or the visit function
         * indicates that this object is the correct one, set
         * the pointer into the private structure and return
         * 1 to stop the search
         */
        if (hfp->visit == NULL || hfp->visit(e, hfp->p) != 0) {
            hfp->e = e;
            return 1;
        }
    }

    return 0;
}

void * cstl_hash_find(struct cstl_hash * const h, const size_t k,
                      cstl_const_visit_func_t * const visit, void * const p)
{
    struct cstl_hash_find_priv hfp;

    hfp.h = h;
    hfp.k = k;
    hfp.visit = visit;
    hfp.p = p;
    hfp.e = NULL;

    cstl_hash_bucket_foreach(
        h, cstl_hash_get_bucket(h, k)->n, cstl_hash_find_visit, &hfp);
    return (void *)hfp.e;
}

/*! @private */
struct cstl_hash_erase_priv
{
    /*
     * initially a pointer to the bucket. as the
     * search progresses, this is a pointer to the
     * pointer to the current object. maintaining
     * this pointer allows the current object to
     * be unlinked from the list if/when it's found
     */
    struct cstl_hash_node ** n;
    /*
     * a pointer to the object being sought, the
     * object that will be removed from the hash
     */
    const void * e;
};

/*! @private */
static int cstl_hash_erase_visit(void * const e, void * const p)
{
    struct cstl_hash_erase_priv * const hep = p;

    if (hep->e == e) {
        return 1;
    }

    /* not found; advance bucket pointer */
    hep->n = &(*hep->n)->next;
    return 0;
}

void cstl_hash_erase(struct cstl_hash * const h, void * const e)
{
    /*
     * the object's key determines which
     * bucket to search for the object
     */
    struct cstl_hash_bucket * const bk =
        cstl_hash_get_bucket(h, __cstl_hash_node(h, e)->key);
    struct cstl_hash_erase_priv hep;

    hep.n = &bk->n;
    hep.e = e;

    /*
     * if the foreach function returns non-zero, then
     * the object was found and removed.
     */
    if (cstl_hash_bucket_foreach(
            h, bk->n, cstl_hash_erase_visit, &hep) != 0) {
        /* object found; splice it out of the list */
        *hep.n = (*hep.n)->next;
        h->count--;
    }
}

/*! @private */
struct cstl_hash_clear_priv
{
    cstl_xtor_func_t * clr;
    void * priv;
};

/*! @private */
static int cstl_hash_clear_visit(void * const e, void * const p)
{
    struct cstl_hash_clear_priv * const hcp = p;
    hcp->clr(e, hcp->priv);
    return 0;
}

void cstl_hash_clear(struct cstl_hash * const h, cstl_xtor_func_t * const clr)
{
    if (clr != NULL) {
        struct cstl_hash_clear_priv hcp;

        /* call the caller's clear function for each object */
        hcp.clr = clr;
        hcp.priv = NULL;
        __cstl_hash_foreach(h, cstl_hash_clear_visit, &hcp);
    }

    free(h->bucket.at);
    h->bucket.at = NULL;

    h->bucket.count = 0;
    h->bucket.capacity = 0;

    h->bucket.rh.hash = NULL;

    h->count = 0;
}

#ifdef __cstl_cfg_test__
// GCOV_EXCL_START
#include "internal/check.h"

#include <stdlib.h>

struct integer {
    int v;
    struct cstl_hash_node n;
};

void __test__cstl_hash_fill(struct cstl_hash * const h, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * in = malloc(sizeof(*in));

        in->v = i;
        cstl_hash_insert(h, in->v, in);
    }

    ck_assert_uint_eq(n, cstl_hash_size(h));
}

static void __test_cstl_hash_free(void * const p, void * const x)
{
    (void)x;
    free(p);
}

static int fill_visit(const void * const e, void * const p)
{
    (void)e;
    *(int *)p += 1;
    return 0;
}

START_TEST(fill)
{
    static const size_t n = 10;
    int c;

    DECLARE_CSTL_HASH(h, struct integer, n);
    cstl_hash_resize(&h, 32, NULL);

    __test__cstl_hash_fill(&h, n);

    c = 0;
    cstl_hash_foreach_const(&h, fill_visit, &c);
    ck_assert_uint_eq(c, n);

    cstl_hash_clear(&h, __test_cstl_hash_free);
}

static int manual_clear_visit(void * const e, void * const p)
{
    cstl_hash_erase((struct cstl_hash *)p, e);
    free(e);
    return 0;
}

START_TEST(manual_clear)
{
    static const size_t n = 10;

    DECLARE_CSTL_HASH(h, struct integer, n);
    cstl_hash_resize(&h, 32, NULL);
    __test__cstl_hash_fill(&h, n);

    cstl_hash_foreach(&h, manual_clear_visit, &h);
    cstl_hash_clear(&h, NULL);
}

static size_t bad_hash_func(const size_t k, const size_t m)
{
    (void)k;
    /*
     * this function is supposed to return a
     * value from [0, m), so returning m should
     * cause an abort()
     */
    return m;
}

START_TEST(bad_hash)
{
    DECLARE_CSTL_HASH(h, struct integer, n);
    cstl_hash_resize(&h, 32, bad_hash_func);
    ck_assert_signal(SIGABRT, cstl_hash_find(&h, 0, NULL, NULL));
    cstl_hash_clear(&h, __test_cstl_hash_free);
}

static void test_rehash(struct cstl_hash * const h,
                        const size_t maxk, const size_t count)
{
    unsigned int i;

    /* all buckets should be dirty following a resize */
    for (i = 0; i < h->bucket.count; i++) {
        ck_assert_uint_ne(h->bucket.at[i].cst, h->bucket.cst);
    }

    /* perform operations on the table until the rehash is complete */
    while (h->bucket.rh.hash != NULL) {
        cstl_hash_find(h, rand() % maxk, NULL, NULL);
    }

    /* check that the table has the correct number of buckets */
    ck_assert_uint_eq(h->bucket.count, count);

    /* all valid buckets should be clean */
    for (i = 0; i < h->bucket.count; i++) {
        ck_assert_uint_eq(h->bucket.at[i].cst, h->bucket.cst);
    }

    /* excess buckets should be empty */
    for (; i < h->bucket.capacity; i++) {
        ck_assert_ptr_null(h->bucket.at[i].n);
    }
}

START_TEST(resize)
{
    static const size_t n = 100;
    const int i = rand() % n;
    bool cst;

    DECLARE_CSTL_HASH(h, struct integer, n);
    void * e;

    cstl_hash_resize(&h, 16, cstl_hash_mul);
    __test__cstl_hash_fill(&h, n);

    e = cstl_hash_find(&h, i, NULL, NULL);
    ck_assert_ptr_ne(e, NULL);

    cst = h.bucket.cst;
    cstl_hash_resize(&h, 16, cstl_hash_mul);
    ck_assert_uint_eq(cst, h.bucket.cst);
    cstl_hash_resize(&h, 16, NULL);
    ck_assert_uint_eq(cst, h.bucket.cst);
    ck_assert_float_eq_tol(cstl_hash_load(&h), (float)n/16, .01f);

    cstl_hash_resize(&h, 20, NULL);
    /* should use the new size even though rehash isn't complete */
    ck_assert_float_eq_tol(cstl_hash_load(&h), (float)n/20, .01f);
    ck_assert_uint_ne(cst, h.bucket.cst);
    cstl_hash_rehash(&h);
    ck_assert_uint_eq(cstl_hash_size(&h), n);
    ck_assert_ptr_eq(e, cstl_hash_find(&h, i, NULL, NULL));

    cstl_hash_resize(&h, 9, cstl_hash_div);
    test_rehash(&h, n, h.bucket.rh.count);
    ck_assert_uint_eq(cstl_hash_size(&h), n);
    ck_assert_ptr_eq(e, cstl_hash_find(&h, i, NULL, NULL));

    cstl_hash_resize(&h, 23, cstl_hash_mul);
    test_rehash(&h, n, h.bucket.rh.count);
    ck_assert_uint_eq(cstl_hash_size(&h), n);
    ck_assert_ptr_eq(e, cstl_hash_find(&h, i, NULL, NULL));

    cstl_hash_resize(&h, 12, cstl_hash_mul);
    cstl_hash_shrink_to_fit(&h);
    ck_assert_ptr_null((void *)(uintptr_t)h.bucket.rh.hash);
    ck_assert_uint_eq(h.bucket.count, 12);
    ck_assert_uint_eq(h.bucket.count, h.bucket.capacity);
    ck_assert_uint_eq(cstl_hash_size(&h), n);
    ck_assert_ptr_eq(e, cstl_hash_find(&h, i, NULL, NULL));

    cstl_hash_erase(&h, e);
    free(e);
    ck_assert_ptr_eq(NULL, cstl_hash_find(&h, i, NULL, NULL));
    ck_assert_uint_eq(cstl_hash_size(&h), n - 1);

    cstl_hash_clear(&h, __test_cstl_hash_free);
}

Suite * hash_suite(void)
{
    Suite * const s = suite_create("hash");

    TCase * tc;

    tc = tcase_create("hash");
    tcase_add_test(tc, fill);
    tcase_add_test(tc, manual_clear);
    tcase_add_test(tc, bad_hash);
    tcase_add_test(tc, resize);
    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
