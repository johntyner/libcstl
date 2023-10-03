#include "hash.h"

#include <stdint.h>

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

static void * __hash_element(const struct hash * const h,
                             struct slist_node * const n)
{
    return (void *)((uintptr_t)n - (h->off + offsetof(struct hash_node, n)));
}

static struct hash_node * __hash_node(const struct hash * const h,
                                      const void * const e)
{
    return (void *)((uintptr_t)e + h->off);
}

static struct slist_node ** hash_bucket(
    struct hash * const h, const unsigned long k)
{
    return vector_at(&h->v, h->hash(k, vector_size(&h->v)));
}

static int hash_bucket_walk(
    struct hash * const h, struct slist_node * n,
    int (* const visit)(void *, void *), void * const p)
{
    int res = 0;

    while (n != NULL && res == 0) {
        struct slist_node * const nn = n->n;
        res = visit(__hash_element(h, n), p);
        n = nn;
    }

    return res;
}

int hash_walk(struct hash * const h,
              int (* const visit)(void *, void *), void * const p)
{
    int res;
    unsigned int i;

    for (i = 0, res = 0; i < vector_size(&h->v) && res == 0; i++) {
        res = hash_bucket_walk(
            h, *(struct slist_node **)vector_at(&h->v, i), visit, p);
    }

    return res;
}

struct hash_resize_priv
{
    struct hash * oh, * nh;
};

static int hash_resize_visit(void * const e, void * const p)
{
    struct hash_resize_priv * const hrp = p;
    const unsigned long k = __hash_node(hrp->oh, e)->k;

    hash_erase(hrp->oh, e);
    hash_insert(hrp->nh, k, e);

    return 0;
}

void hash_resize(struct hash * const h,
                 const size_t n, size_t (* hash)(unsigned long, size_t))
{
    if (n > 0) {
        struct hash_resize_priv hrp;
        struct hash h2;
        unsigned int i;

        hash_init(&h2, h->off);
        vector_resize(&h2.v, n);
        if (hash == NULL) {
            h2.hash = hash_mul;
        } else {
            h2.hash = hash;
        }

        for (i = 0; i < n; i++) {
            *(struct slist_node **)vector_at(&h2.v, i) = NULL;
        }

        hrp.oh = h;
        hrp.nh = &h2;

        hash_walk(h, hash_resize_visit, &hrp);
        hash_swap(h, &h2);
        hash_clear(&h2, NULL);
    }
}

void hash_insert(struct hash * const h,
                 const unsigned long k, void * const e)
{
    struct slist_node ** const sn = hash_bucket(h, k);
    struct hash_node * const hn = __hash_node(h, e);

    hn->k = k;
    hn->n.n = *sn;
    *sn = &hn->n;

    h->count++;
}

struct hash_find_priv
{
    struct hash * h;
    unsigned long k;
    void * e;
    int (* visit)(const void *, void *);
    void * p;
};

static int hash_find_visit(void * const e, void * const p)
{
    struct hash_find_priv * const hfp = p;

    if (__hash_node(hfp->h, e)->k == hfp->k) {
        if (hfp->visit == NULL || hfp->visit(e, hfp->p) != 0) {
            hfp->e = e;
            return 1;
        }
    }

    return 0;
}

void * hash_find(struct hash * const h, const unsigned long k,
                 int (* const visit)(const void *, void *), void * const p)
{
    struct hash_find_priv hfp;

    hfp.h = h;
    hfp.k = k;
    hfp.e = NULL;
    hfp.visit = visit;
    hfp.p = p;

    hash_bucket_walk(h, *hash_bucket(h, k), hash_find_visit, &hfp);
    return hfp.e;
}

struct hash_erase_priv
{
    struct slist_node ** n;
    void * e;
};

static int hash_erase_visit(void * const e, void * const p)
{
    struct hash_erase_priv * const hep = p;

    if (hep->e == e) {
        *hep->n = (*hep->n)->n;
        return 1;
    }

    hep->n = &(*hep->n)->n;
    return 0;
}

void hash_erase(struct hash * const h, void * const e)
{
    struct hash_erase_priv hep;

    hep.n = hash_bucket(h, __hash_node(h, e)->k);
    hep.e = e;

    if (hash_bucket_walk(h, *hep.n, hash_erase_visit, &hep) != 0) {
        h->count--;
    }
}

void hash_swap(struct hash * const h1, struct hash * const h2)
{
    size_t (* tf)(unsigned long, size_t);
    size_t t;

    vector_swap(&h1->v, &h2->v);
    cstl_swap(&h1->count, &h2->count, &t, sizeof(t));
    cstl_swap(&h1->off, &h2->off, &t, sizeof(t));

    tf = h1->hash;
    h1->hash = h2->hash;
    h2->hash = tf;
}

struct hash_clear_priv
{
    void (* clr)(void *);
};

static int hash_clear_visit(void * const e, void * const p)
{
    struct hash_clear_priv * const hcp = p;
    hcp->clr(e);
    return 0;
}

void hash_clear(struct hash * const h, void (* const clr)(void *))
{
    struct hash_clear_priv hcp;

    hcp.clr = clr;
    hash_walk(h, hash_clear_visit, &hcp);

    vector_clear(&h->v);
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
        struct integer * in = malloc(sizeof(*in));

        in->v = i;
        hash_insert(h, in->v, in);
    }

    ck_assert_uint_eq(n, hash_size(h));
}

START_TEST(fill)
{
    static const size_t n = 10;

    struct hash h;

    HASH_INIT(&h, struct integer, n);
    hash_resize(&h, 32, hash_mul);

    __test__hash_fill(&h, n);

    hash_clear(&h, free);
}

START_TEST(resize)
{
    static const size_t n = 100;
    const int i = rand() % n;

    struct hash h;
    void * e;

    HASH_INIT(&h, struct integer, n);
    hash_resize(&h, 16, hash_mul);

    __test__hash_fill(&h, n);

    e = hash_find(&h, i, NULL, NULL);
    ck_assert_ptr_ne(e, NULL);

    hash_resize(&h, 9, hash_div);
    ck_assert_uint_eq(hash_size(&h), n);
    ck_assert_ptr_eq(e, hash_find(&h, i, NULL, NULL));

    hash_resize(&h, 23, hash_mul);
    ck_assert_uint_eq(hash_size(&h), n);
    ck_assert_ptr_eq(e, hash_find(&h, i, NULL, NULL));

    hash_erase(&h, e);
    free(e);
    ck_assert_ptr_eq(NULL, hash_find(&h, i, NULL, NULL));
    ck_assert_uint_eq(hash_size(&h), n - 1);

    hash_clear(&h, free);
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
