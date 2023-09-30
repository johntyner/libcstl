#include "heap.h"
#include "common.h"

#include <assert.h>
#include <stdint.h>

/*
 * each node in the tree is associated with a numerical identifier with
 * the root being 0. each child node is assigned the value of 2 times
 * its parent's id plus 1 for the left child and plus 2 for the right.
 *
 * ex: 0 is the root node. it's children are 1 (left) and 2 (right).
 *     1's children are 3 (left) and 4 (right).
 *
 * ---
 *
 * often, we'll want to find a particular node in the tree by it's id.
 * since the tree is a binary tree, the 1's and 0's of the binary
 * representation of the id can be used to navigate the tree. the issue
 * is that the bits need to be read from msb to lsb, and it's not obvious
 * how many bits represent the id.
 *
 * to solve this, we add 1 to the id. now the highest set bit, tells us
 * the number of bits we're dealing with and the remaining bits tell us
 * to go left (0) or right (1) down the tree to find the particular node.
 */

static struct bintree_node * heap_find(struct bintree_node * p,
                                       const unsigned int id)
{
    if (id > 0) {
        const unsigned int loc = id + 1;
        unsigned int b;

        for (b = 1 << (cstl_fls(loc) - 1); b != 0; b >>= 1) {
            if ((loc & b) == 0) {
                p = p->l;
            } else {
                p = p->r;
            }
        }
    }

    return p;
}

static void heap_swap(struct bintree * const h,
                      struct bintree_node * const p,
                      struct bintree_node * const c)
{
    struct bintree_node t;

    assert(c->p == p);

    /* swap parent and child */
    t = *p;
    *p = *c;
    *c = t;


    /* one of c's children is now itself */
    assert(c->l == c || c->r == c);
    if (c->l == c) {
        c->l = p;
    } else {
        c->r = p;
    }

    /* p is its own parent */
    assert(p->p == p);
    /*
     * since p is now one of c's children,
     * one of these will fix p's parentage.
     */
    if (c->l != NULL) {
        c->l->p = c;
    }
    if (c->r != NULL) {
        c->r->p = c;
    }

    /*
     * c's new parent needs to be made to
     * point to c as one of its children
     */
    if (c->p == NULL) {
        h->root = c;
    } else if (c->p->l == p) {
        c->p->l = c;
    } else {
        c->p->r = c;
    }

    /* c's previous childrens' parent is now p */
    if (p->l != NULL) {
        assert(p->l->p == c);
        p->l->p = p;
    }
    if (p->r != NULL) {
        assert(p->r->p == c);
        p->r->p = p;
    }
}

void heap_push(struct bintree * const h, void * const p)
{
    struct bintree_node * const n = (void *)((uintptr_t)p + h->off);

    n->l = NULL;
    n->r = NULL;

    if (h->root == NULL) {
        n->p = NULL;
        h->root = n;
    } else {
        n->p = heap_find(h->root, (h->size - 1) / 2);

        if (h->size % 2 == 0) {
            n->p->r = n;
        } else {
            n->p->l = n;
        }

        /* bubble n up through the tree to its correct spot */
        while (n->p != NULL && bintree_cmp(h, n, n->p) > 0) {
            heap_swap(h, n->p, n);
        }
    }

    h->size++;
}

const void * heap_get(const struct bintree * const h)
{
    if (h->root != NULL) {
        return (void *)((uintptr_t)h->root - h->off);
    }

    return NULL;
}

void * heap_pop(struct bintree * const h)
{
    void * const res = (void *)heap_get(h);

    if (res != NULL) {
        struct bintree_node * n;

        n = heap_find(h->root, h->size - 1);
        assert(n->l == NULL && n->r == NULL);

        if (n->p == NULL) {
            h->root = NULL;
        } else if (n->p->l == n) {
            n->p->l = NULL;
        } else {
            n->p->r = NULL;
        }

        h->size--;

        if (h->root != NULL) {
            *n = *h->root;
            if (n->l != NULL) {
                n->l->p = n;
            }
            if (n->r != NULL) {
                n->r->p = n;
            }

            h->root = n;

            while ((n->l != NULL && bintree_cmp(h, n->l, n) > 0)
                   || (n->r != NULL && bintree_cmp(h, n->r, n) > 0)) {
                struct bintree_node * c;

                if (n->l == NULL) {
                    c = n->r;
                } else if (n->r == NULL) {
                    c = n->l;
                } else if (bintree_cmp(h, n->l, n->r) > 0) {
                    c = n->l;
                } else {
                    c = n->r;
                }

                heap_swap(h, n, c);
            }
        }

    }

    return res;
}

#ifdef __cfg_test__
#include <check.h>
#include <stdlib.h>
#include <limits.h>

static int __heap_verify(const struct bintree_node * const bn,
                         const bintree_visit_order_t order,
                         void * const priv)
{
    if (order == BINTREE_VISIT_ORDER_MID
        || order == BINTREE_VISIT_ORDER_LEAF) {
        const struct bintree * const h = priv;

        if (bn->l != NULL) {
            ck_assert_int_le(bintree_cmp(h, bn->l, bn), 0);
        }
        if (bn->r != NULL) {
            ck_assert_int_le(bintree_cmp(h, bn->r, bn), 0);
        }
    }

    return 0;
}

static void heap_verify(const struct bintree * const h)
{
    if (h->root != NULL) {
        size_t min, max;

        __bintree_walk(h->root, __heap_verify, (void *)h,
                       __bintree_left, __bintree_right);

        bintree_height(h, &min, &max);

        /*
         * the tree should always be as compact as
         * possible. in fact, we could go a step further
         * to make sure there are no gaps in the leaves
         */

        ck_assert_uint_le(max - min, 1);
        ck_assert_uint_le(max, log2(bintree_size(h)) + 1);
    }
}

struct integer {
    int v;
    struct bintree_node bn;
};

static int cmp_integer(const void * const a, const void * const b)
{
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

static void __test__heap_fill(struct bintree * const h, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * const in = malloc(sizeof(*in));

        in->v = rand() % n;

        heap_push(h, in);
        ck_assert_uint_eq(i + 1, bintree_size(h));
    }
}

static void __test__heap_drain(struct bintree * const h)
{
    size_t sz;
    int n;

    n = INT_MAX;
    while ((sz = bintree_size(h)) > 0) {
        struct integer * in = heap_pop(h);

        ck_assert_int_le(in->v, n);
        free(in);

        ck_assert_uint_eq(sz - 1, bintree_size(h));

        heap_verify(h);
    }

    ck_assert_ptr_null(h->root);
    ck_assert_uint_eq(bintree_size(h), 0);
}

START_TEST(fill)
{
    static const size_t n = 100;

    struct bintree h;

    HEAP_INIT(&h, struct integer, bn, cmp_integer);
    __test__heap_fill(&h, n);
    heap_verify(&h);
    __test__heap_drain(&h);
}
END_TEST

Suite * heap_suite(void)
{
    Suite * const s = suite_create("heap");

    TCase * tc;

    tc = tcase_create("heap");
    tcase_add_test(tc, fill);
    suite_add_tcase(s, tc);

    return s;
}

#endif
