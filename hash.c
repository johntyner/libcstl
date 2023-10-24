/*!
 * @file
 */

#include "hash.h"

unsigned long hash_div(const unsigned long k, const size_t m)
{
    return k % m;
}

unsigned long hash_mul(const unsigned long k, const size_t m)
{
    static const float phi = 1.618034;
    const float M = phi * k;
    return (M - (unsigned long)M) * m;
}

/*!
 * @private
 *
 * Given a hash_node, return the containing object
 */
static void * __hash_element(const struct hash * const h,
                             struct hash_node * const n)
{
    return (void *)((uintptr_t)n - h->off);
}

/*!
 * @private
 *
 * Given an object, return the contained hash_node
 */
static struct hash_node * __hash_node(const struct hash * const h,
                                      const void * const e)
{
    return (void *)((uintptr_t)e + h->off);
}

/*!
 * @private
 *
 * Given a key, return the associated hash bucket
 */
static struct hash_node ** hash_bucket(
    struct hash * const h, const unsigned long k)
{
    return &h->b.v[h->hash(k, h->b.n)];
}

/*!
 * @private
 *
 * Visit each element within a particular bucket in the hash table
 */
static int hash_bucket_foreach(
    struct hash * const h, struct hash_node * n,
    cstl_visit_func_t * const visit, void * const p)
{
    int res = 0;

    while (n != NULL && res == 0) {
        struct hash_node * const nn = n->n;
        res = visit(__hash_element(h, n), p);
        n = nn;
    }

    return res;
}

int hash_foreach(struct hash * const h,
                 cstl_visit_func_t * const visit, void * const p)
{
    int res;
    unsigned int i;

    for (i = 0, res = 0; i < h->b.n && res == 0; i++) {
        res = hash_bucket_foreach(h, h->b.v[i], visit, p);
    }

    return res;
}

/*! @private */
static int hash_resize_visit(void * const e, void * const p)
{
    /* p points to the new hash table */
    struct hash * const nh = p;
    /*
     * even though e is in the old hash table, the @off member
     * of the new hash table is the same, and it can be used
     * to extract the hash node and key from the element
     */
    const unsigned long k = __hash_node(nh, e)->k;

    /*
     * don't bother removing the element from the old table.
     * the insertion will overwrite the pointers. it's up to
     * the caller to clear the buckets, knowing that the
     * objects contained in them are no longer there.
     */
    hash_insert(nh, k, e);

    return 0;
}

void hash_resize(struct hash * const h,
                 struct hash_node ** v, const size_t n,
                 hash_func_t * const hash)
{
    if (n > 0) {
        /*
         * can't DECLARE_HASH because the type
         * of object is unknown. use the offset
         * to init
         */
        struct hash h2;
        hash_init(&h2, h->off);

        /*
         * create the buckets. they may have been
         * supplied by the caller; note that if
         * necessary
         */
        h2.b.ext = (v != NULL);
        if (h2.b.ext) {
            h2.b.v = v;
        } else {
            h2.b.v = cstl_malloc(sizeof(*h2.b.v) * n);
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
                h2.hash = hash_mul;
            }

            /*
             * walk through all nodes in the current table
             * and reinsert them into the new table with
             * the new hash function. keys and everything
             * else remains the same
             */
            hash_foreach(h, hash_resize_visit, &h2);

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
            hash_clear(h, NULL);

            /* move the new hash table to the caller's object */
            hash_swap(h, &h2);
        } /*
           * else, allocation of buckets failed.
           * nothing to clean up; just exit
           */
    }
}

void hash_insert(struct hash * const h,
                 const unsigned long k, void * const e)
{
    struct hash_node ** const bk = hash_bucket(h, k);
    struct hash_node * const hn = __hash_node(h, e);

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
struct hash_find_priv
{
    /* the hash, obviously */
    struct hash * h;
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
static int hash_find_visit(void * const e, void * const p)
{
    struct hash_find_priv * const hfp = p;

    /* only visit nodes with matching keys */
    if (__hash_node(hfp->h, e)->k == hfp->k) {
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

void * hash_find(struct hash * const h, const unsigned long k,
                 cstl_const_visit_func_t * const visit, void * const p)
{
    struct hash_find_priv hfp;

    hfp.h = h;
    hfp.k = k;
    hfp.visit = visit;
    hfp.p = p;
    hfp.e = NULL;

    hash_bucket_foreach(h, *hash_bucket(h, k), hash_find_visit, &hfp);
    return hfp.e;
}

/*! @private */
struct hash_erase_priv
{
    /*
     * initially a pointer to the bucket. as the
     * search progresses, this is a pointer to the
     * pointer to the current object. maintaining
     * this pointer allows the current object to
     * be unlinked from the list if/when it's found
     */
    struct hash_node ** n;
    /*
     * a pointer to the object being sought, the
     * object that will be removed from the hash
     */
    void * e;
};

/*! @private */
static int hash_erase_visit(void * const e, void * const p)
{
    struct hash_erase_priv * const hep = p;

    if (hep->e == e) {
        /* object found; splice it out of the list */
        *hep->n = (*hep->n)->n;
        return 1;
    }

    /* not found; advance bucket pointer */
    hep->n = &(*hep->n)->n;
    return 0;
}

void hash_erase(struct hash * const h, void * const e)
{
    struct hash_erase_priv hep;

    /*
     * the object's key determines which
     * bucket to search for the object
     */
    hep.n = hash_bucket(h, __hash_node(h, e)->k);
    hep.e = e;

    /*
     * if the foreach function returns non-zero, then
     * the object was found and removed.
     */
    if (hash_bucket_foreach(h, *hep.n, hash_erase_visit, &hep) != 0) {
        h->count--;
    }
}

/*! @private */
struct hash_clear_priv
{
    cstl_clear_func_t * clr;
};

/*! @private */
static int hash_clear_visit(void * const e, void * const p)
{
    struct hash_clear_priv * const hcp = p;
    hcp->clr(e, NULL);
    return 0;
}

void hash_clear(struct hash * const h, cstl_clear_func_t * const clr)
{
    if (clr != NULL) {
        struct hash_clear_priv hcp;

        /* call the caller's clear function for each object */
        hcp.clr = clr;
        hash_foreach(h, hash_clear_visit, &hcp);
    }

    if (!h->b.ext) {
        /*
         * only free the buckets if they
         * were allocated internally
         */
        cstl_free(h->b.v);
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
    struct hash_node n;
};

static int cmp_integer(const void * const a, const void * const b)
{
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

void __test__hash_fill(struct hash * const h, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * in = cstl_malloc(sizeof(*in));

        in->v = i;
        hash_insert(h, in->v, in);
    }

    ck_assert_uint_eq(n, hash_size(h));
}

static void __test_hash_free(void * const p, void * const x)
{
    (void)x;
    cstl_free(p);
}

START_TEST(fill)
{
    static const size_t n = 10;

    DECLARE_HASH(h, struct integer, n);
    hash_resize(&h, NULL, 32, hash_mul);

    __test__hash_fill(&h, n);

    hash_clear(&h, __test_hash_free);
}

START_TEST(resize)
{
    static const size_t n = 100;
    const int i = cstl_rand() % n;

    DECLARE_HASH(h, struct integer, n);
    void * e;

    hash_resize(&h, NULL, 16, hash_mul);

    __test__hash_fill(&h, n);

    e = hash_find(&h, i, NULL, NULL);
    ck_assert_ptr_ne(e, NULL);

    hash_resize(&h, NULL, 9, hash_div);
    ck_assert_uint_eq(hash_size(&h), n);
    ck_assert_ptr_eq(e, hash_find(&h, i, NULL, NULL));

    hash_resize(&h, NULL, 23, hash_mul);
    ck_assert_uint_eq(hash_size(&h), n);
    ck_assert_ptr_eq(e, hash_find(&h, i, NULL, NULL));

    hash_erase(&h, e);
    cstl_free(e);
    ck_assert_ptr_eq(NULL, hash_find(&h, i, NULL, NULL));
    ck_assert_uint_eq(hash_size(&h), n - 1);

    hash_clear(&h, __test_hash_free);
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
