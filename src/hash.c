/*!
 * @file
 */

#include "cstl/hash.h"

#include <stdlib.h>

unsigned long cstl_hash_div(const unsigned long k, const size_t m)
{
    return k % m;
}

unsigned long cstl_hash_mul(const unsigned long k, const size_t m)
{
    static const float phi = 1.618034;
    const float M = phi * k;
    return (M - (unsigned long)M) * m;
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

static struct cstl_hash_node ** __cstl_hash_bucket(
    struct cstl_hash * const h, const unsigned long k,
    cstl_hash_func_t * const hash, const size_t count)
{
    return &h->bucket.n[hash(k, count)];
}

static void cstl_clean_bucket(
    struct cstl_hash * const h, struct cstl_hash_node ** const head)
{
    struct cstl_hash_node * n;

    n = *head;
    *head = NULL;

    while (n != NULL) {
        struct cstl_hash_node * const nn = n->next;
        struct cstl_hash_node ** const b =
            __cstl_hash_bucket(h, n->key,
                               h->bucket.rh.hash, h->bucket.rh.count);

        n->next = *b;
        *b = n;

        n = nn;
    }
}

/*!
 * @private
 *
 * Given a key, return the associated hash bucket
 */
static struct cstl_hash_node ** cstl_hash_bucket(
    struct cstl_hash * const h, const unsigned long k)
{
    struct cstl_hash_node ** n;

    n = __cstl_hash_bucket(h, k, h->bucket.hash, h->bucket.count);

    if (h->bucket.rh.hash != NULL) {
        struct cstl_hash_node ** const _n =
            __cstl_hash_bucket(h, k, h->bucket.rh.hash, h->bucket.rh.count);

        /*!
         * @todo: This is broken: because there is no way to mark
         * a bucket as clean, it's going to take n calls to this
         * function to clean them all, and many of them will be
         * cleaned multiple times. it'll work, but it's wildly
         * inefficient.
         */

        cstl_clean_bucket(h, n);
        cstl_clean_bucket(h, _n);

        cstl_clean_bucket(h, &h->bucket.n[h->bucket.rh.clean]);
        h->bucket.rh.clean++;

        if (h->bucket.rh.clean >= h->bucket.count &&
            h->bucket.rh.clean >= h->bucket.rh.count) {
            h->bucket.count = h->bucket.rh.count;
            h->bucket.hash = h->bucket.rh.hash;

            h->bucket.rh.hash = NULL;
        }

        n = _n;
    }

    return n;
}

/*!
 * @private
 *
 * Visit each element within a particular bucket in the hash table
 */
static int cstl_hash_bucket_foreach(
    struct cstl_hash * const h, struct cstl_hash_node * n,
    cstl_visit_func_t * const visit, void * const p)
{
    int res = 0;

    while (n != NULL && res == 0) {
        struct cstl_hash_node * const nn = n->next;
        res = visit(__cstl_hash_element(h, n), p);
        n = nn;
    }

    return res;
}

int cstl_hash_foreach(struct cstl_hash * const h,
                      cstl_visit_func_t * const visit, void * const p)
{
    int res;
    unsigned int i;

    for (i = 0, res = 0; i < h->bucket.count && res == 0; i++) {
        res = cstl_hash_bucket_foreach(h, h->bucket.n[i], visit, p);
    }

    return res;
}

void cstl_hash_resize(struct cstl_hash * const h,
                      const size_t count, cstl_hash_func_t * const hash)
{
    if (h->bucket.rh.hash == NULL) {
        if (count > h->bucket.capacity) {
            struct cstl_hash_node ** const n =
                realloc(h->bucket.n, sizeof(*n) * count);
            if (n != NULL) {
                h->bucket.n = n;
                h->bucket.capacity = count;
            }
        }

        if (h->bucket.n != NULL && count <= h->bucket.capacity) {
            unsigned int i;

            /*
             * buckets successfully allocated. initialize them.
             */
            for (i = h->bucket.count; i < count; i++) {
                h->bucket.n[i] = NULL;
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

void cstl_hash_insert(struct cstl_hash * const h,
                      const unsigned long k, void * const e)
{
    struct cstl_hash_node ** const n = cstl_hash_bucket(h, k);
    struct cstl_hash_node * const hn = __cstl_hash_node(h, e);

    hn->key = k;

    /*
     * the bucket is a singly-linked/forward list.
     * insert the new object at the front of the list.
     */
    hn->next = *n;
    *n = hn;

    h->count++;
}

/*! @private */
struct cstl_hash_find_priv
{
    /* the hash, obviously */
    struct cstl_hash * h;
    /* the key being sought */
    unsigned long k;
    cstl_const_visit_func_t * visit;
    void * p;
    /*
     * initially null, set to the found object
     * if the visit function indicates that it
     * is the object being sought
     */
    void * e;
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

void * cstl_hash_find(struct cstl_hash * const h, const unsigned long k,
                      cstl_const_visit_func_t * const visit, void * const p)
{
    struct cstl_hash_find_priv hfp;

    hfp.h = h;
    hfp.k = k;
    hfp.visit = visit;
    hfp.p = p;
    hfp.e = NULL;

    cstl_hash_bucket_foreach(
        h, *cstl_hash_bucket(h, k), cstl_hash_find_visit, &hfp);
    return hfp.e;
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
    void * e;
};

/*! @private */
static int cstl_hash_erase_visit(void * const e, void * const p)
{
    struct cstl_hash_erase_priv * const hep = p;

    if (hep->e == e) {
        /* object found; splice it out of the list */
        *hep->n = (*hep->n)->next;
        return 1;
    }

    /* not found; advance bucket pointer */
    hep->n = &(*hep->n)->next;
    return 0;
}

void cstl_hash_erase(struct cstl_hash * const h, void * const e)
{
    struct cstl_hash_erase_priv hep;
    struct cstl_hash_node ** const n =
        cstl_hash_bucket(h, __cstl_hash_node(h, e)->key);

    /*
     * the object's key determines which
     * bucket to search for the object
     */
    hep.n = n;
    hep.e = e;

    /*
     * if the foreach function returns non-zero, then
     * the object was found and removed.
     */
    if (cstl_hash_bucket_foreach(
            h, *n, cstl_hash_erase_visit, &hep) != 0) {
        h->count--;
    }
}

/*! @private */
struct cstl_hash_clear_priv
{
    cstl_xtor_func_t * clr;
};

/*! @private */
static int cstl_hash_clear_visit(void * const e, void * const p)
{
    struct cstl_hash_clear_priv * const hcp = p;
    hcp->clr(e, NULL);
    return 0;
}

void cstl_hash_clear(struct cstl_hash * const h, cstl_xtor_func_t * const clr)
{
    if (clr != NULL) {
        struct cstl_hash_clear_priv hcp;

        /* call the caller's clear function for each object */
        hcp.clr = clr;
        cstl_hash_foreach(h, cstl_hash_clear_visit, &hcp);
    }

    free(h->bucket.n);
    h->bucket.n = NULL;
    h->bucket.count = 0;

    h->count = 0;
}

#ifdef __cfg_test__
// GCOV_EXCL_START
#include <check.h>

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

START_TEST(fill)
{
    static const size_t n = 10;

    DECLARE_CSTL_HASH(h, struct integer, n);
    cstl_hash_resize(&h, 32, NULL);

    __test__cstl_hash_fill(&h, n);

    cstl_hash_clear(&h, __test_cstl_hash_free);
}

START_TEST(resize)
{
    static const size_t n = 100;
    const int i = rand() % n;

    DECLARE_CSTL_HASH(h, struct integer, n);
    void * e;

    cstl_hash_resize(&h, 16, cstl_hash_mul);

    __test__cstl_hash_fill(&h, n);

    e = cstl_hash_find(&h, i, NULL, NULL);
    ck_assert_ptr_ne(e, NULL);

    cstl_hash_resize(&h, 9, cstl_hash_div);
    ck_assert_uint_eq(cstl_hash_size(&h), n);
    ck_assert_ptr_eq(e, cstl_hash_find(&h, i, NULL, NULL));

    cstl_hash_resize(&h, 23, cstl_hash_mul);
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
    tcase_add_test(tc, resize);
    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
