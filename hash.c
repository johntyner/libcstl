/*!
 * @file
 */

#include "hash.h"

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

/*!
 * @private
 *
 * Given a key, return the associated hash bucket
 */
static struct cstl_hash_node ** cstl_hash_bucket(
    struct cstl_hash * const h, const unsigned long k)
{
    return &h->b.v[h->hash(k, h->b.n)];
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
        struct cstl_hash_node * const nn = n->n;
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

    for (i = 0, res = 0; i < h->b.n && res == 0; i++) {
        res = cstl_hash_bucket_foreach(h, h->b.v[i], visit, p);
    }

    return res;
}

/*! @private */
static int cstl_hash_resize_visit(void * const e, void * const p)
{
    /* p points to the new hash table */
    struct cstl_hash * const nh = p;
    /*
     * even though e is in the old hash table, the @off member
     * of the new hash table is the same, and it can be used
     * to extract the hash node and key from the element
     */
    const unsigned long k = __cstl_hash_node(nh, e)->k;

    /*
     * don't bother removing the element from the old table.
     * the insertion will overwrite the pointers. it's up to
     * the caller to clear the buckets, knowing that the
     * objects contained in them are no longer there.
     */
    cstl_hash_insert(nh, k, e);

    return 0;
}

void cstl_hash_resize(struct cstl_hash * const h,
                      struct cstl_hash_node ** v, const size_t n,
                      cstl_hash_func_t * const hash)
{
    if (n > 0) {
        /*
         * can't DECLARE_CSTL_HASH because the type
         * of object is unknown. use the offset
         * to init
         */
        struct cstl_hash h2;
        cstl_hash_init(&h2, h->off);

        /*
         * create the buckets. they may have been
         * supplied by the caller; note that if
         * necessary
         */
        h2.b.ext = (v != NULL);
        if (h2.b.ext) {
            h2.b.v = v;
        } else {
            h2.b.v = malloc(sizeof(*h2.b.v) * n);
        }

        if (h2.b.v != NULL) {
            unsigned int i;

            /*
             * buckets successfully allocated or supplied
             * by the caller. initialize them.
             */
            h2.b.n = n;
            for (i = 0; i < h2.b.n; i++) {
                h2.b.v[i] = NULL;
            }

            /*
             * set the hash function in the new hash table.
             * there is no requirement that it be the same
             * function as the one in the current table
             */
            if (hash != NULL) {
                h2.hash = hash;
            } else if (h->hash != NULL) {
                h2.hash = h->hash;
            } else {
                h2.hash = cstl_hash_mul;
            }

            /*
             * walk through all nodes in the current table
             * and reinsert them into the new table with
             * the new hash function. keys and everything
             * else remains the same
             */
            cstl_hash_foreach(h, cstl_hash_resize_visit, &h2);

            /*
             * the foreach call may not necessarily have
             * cleanly removed the objects from the existing
             * hash (for efficiency purposes). this code
             * needs to clear the buckets so that the clear
             * function doesn't do anything to the removed
             * nodes. it's even faster to just set the number
             * of buckets to zero.
             */
            h->b.n = 0;
            cstl_hash_clear(h, NULL);

            /* move the new hash table to the caller's object */
            cstl_hash_swap(h, &h2);
        } /*
           * else, allocation of buckets failed.
           * nothing to clean up; just exit
           */
    }
}

void cstl_hash_insert(struct cstl_hash * const h,
                      const unsigned long k, void * const e)
{
    struct cstl_hash_node ** const bk = cstl_hash_bucket(h, k);
    struct cstl_hash_node * const hn = __cstl_hash_node(h, e);

    hn->k = k;

    /*
     * the bucket is a singly-linked/forward list.
     * insert the new object at the front of the list.
     */
    hn->n = *bk;
    *bk = hn;

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
    if (__cstl_hash_node(hfp->h, e)->k == hfp->k) {
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
        *hep->n = (*hep->n)->n;
        return 1;
    }

    /* not found; advance bucket pointer */
    hep->n = &(*hep->n)->n;
    return 0;
}

void cstl_hash_erase(struct cstl_hash * const h, void * const e)
{
    struct cstl_hash_erase_priv hep;

    /*
     * the object's key determines which
     * bucket to search for the object
     */
    hep.n = cstl_hash_bucket(h, __cstl_hash_node(h, e)->k);
    hep.e = e;

    /*
     * if the foreach function returns non-zero, then
     * the object was found and removed.
     */
    if (cstl_hash_bucket_foreach(
            h, *hep.n, cstl_hash_erase_visit, &hep) != 0) {
        h->count--;
    }
}

/*! @private */
struct cstl_hash_clear_priv
{
    cstl_clear_func_t * clr;
};

/*! @private */
static int cstl_hash_clear_visit(void * const e, void * const p)
{
    struct cstl_hash_clear_priv * const hcp = p;
    hcp->clr(e, NULL);
    return 0;
}

void cstl_hash_clear(struct cstl_hash * const h, cstl_clear_func_t * const clr)
{
    if (clr != NULL) {
        struct cstl_hash_clear_priv hcp;

        /* call the caller's clear function for each object */
        hcp.clr = clr;
        cstl_hash_foreach(h, cstl_hash_clear_visit, &hcp);
    }

    if (!h->b.ext) {
        /*
         * only free the buckets if they
         * were allocated internally
         */
        free(h->b.v);
    }

    h->b.ext = 0;
    h->b.v = NULL;
    h->b.n = 0;

    h->count = 0;
    h->hash = NULL;
}

#ifdef __cfg_test__
#include <check.h>

#include <stdlib.h>

struct integer {
    int v;
    struct cstl_hash_node n;
};

static int cmp_integer(const void * const a, const void * const b)
{
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

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
    cstl_hash_resize(&h, NULL, 32, cstl_hash_mul);

    __test__cstl_hash_fill(&h, n);

    cstl_hash_clear(&h, __test_cstl_hash_free);
}

START_TEST(resize)
{
    static const size_t n = 100;
    const int i = rand() % n;

    DECLARE_CSTL_HASH(h, struct integer, n);
    void * e;

    cstl_hash_resize(&h, NULL, 16, cstl_hash_mul);

    __test__cstl_hash_fill(&h, n);

    e = cstl_hash_find(&h, i, NULL, NULL);
    ck_assert_ptr_ne(e, NULL);

    cstl_hash_resize(&h, NULL, 9, cstl_hash_div);
    ck_assert_uint_eq(cstl_hash_size(&h), n);
    ck_assert_ptr_eq(e, cstl_hash_find(&h, i, NULL, NULL));

    cstl_hash_resize(&h, NULL, 23, cstl_hash_mul);
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

#endif
