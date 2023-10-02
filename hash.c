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

struct hash_resize_priv
{
    struct hash * oh, * nh;
};

static struct hash_node * __hash_node(const struct hash * const h,
                                      const void * const e)
{
    return (void *)((uintptr_t)e + h->off);
}

static struct slist * hash_bucket(struct hash * const h, const unsigned long k)
{
    return vector_at(&h->v, h->hash(k, vector_size(&h->v)));
}

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
            slist_init(vector_at(&h2.v, i),
                       h->off + offsetof(struct hash_node, n));
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
    __hash_node(h, e)->k = k;
    slist_push_front(hash_bucket(h, k), e);
    h->count++;
}

struct hash_find_priv
{
    struct hash * h;
    unsigned long k;
    int (* visit)(const void *, void *);
    void * p, * e;
};

static int hash_find_visit(void * const e, void * const p)
{
    struct hash_find_priv * const hfp = p;
    int res = 0;

    if (__hash_node(hfp->h, e)->k == hfp->k) {
        if (hfp->visit == NULL) {
            res = 1;
        } else {
            res = hfp->visit(e, hfp->p);
        }

        if (res > 0) {
            hfp->e = e;
        }
    }

    return res;
}

void * hash_find(struct hash * const h, const unsigned long k,
                   int (* const visit)(const void *, void *), void * const p)
{
    struct hash_find_priv hfp;

    hfp.h = h;
    hfp.k = k;
    hfp.visit = visit;
    hfp.p = p;
    hfp.e = NULL;

    slist_walk(hash_bucket(h, k), hash_find_visit, &hfp);

    return hfp.e;
}

struct hash_erase_priv
{
    struct slist * l;
    void * p, * e;
};

static int hash_erase_visit(void * const e, void * const p)
{
    struct hash_erase_priv * const hep = p;

    if (hep->e == e) {
        if (hep->p == NULL) {
            slist_pop_front(hep->l);
        } else {
            slist_erase_after(hep->l, p);
        }

        return 1;
    }

    hep->p = e;

    return 0;
}

void hash_erase(struct hash * const h, void * const e)
{
    struct hash_erase_priv hep;

    hep.l = hash_bucket(h, __hash_node(h, e)->k);
    hep.p = NULL;
    hep.e = e;

    if (slist_walk(hep.l, hash_erase_visit, &hep) == 1) {
        h->count--;
    }
}

int hash_walk(struct hash * const h,
              int (* const visit)(void *, void *), void * const p)
{
    int res;
    unsigned int i;

    for (i = 0, res = 0;
         i < vector_size(&h->v) && res == 0;
         res = slist_walk(vector_at(&h->v, i), visit, p), i++)
        ;

    return res;
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

void hash_clear(struct hash * const h, void (* const clr)(void *))
{
    unsigned int i;

    for (i = 0; i < vector_size(&h->v); i++) {
        struct slist * const l = vector_at(&h->v, i);
        while (slist_size(l) > 0) {
            clr(slist_pop_front(l));
        }
    }

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

        in->v = rand() % n;
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

    struct hash h;
    int i;
    void * e;

    HASH_INIT(&h, struct integer, n);
    hash_resize(&h, 16, hash_mul);

    __test__hash_fill(&h, n);

    i = 0;
    while ((e = hash_find(&h, ++i, NULL, NULL)) == NULL)
        ;

    hash_resize(&h, 9, hash_mul);
    ck_assert_uint_eq(hash_size(&h), n);
    ck_assert_ptr_eq(e, hash_find(&h, i, NULL, NULL));

    hash_resize(&h, 23, hash_mul);
    ck_assert_uint_eq(hash_size(&h), n);
    ck_assert_ptr_eq(e, hash_find(&h, i, NULL, NULL));

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
