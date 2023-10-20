/*!
 * @file
 */

#include "bintree.h"

/*!
 * @private
 *
 * Given a pointer to an element, return a (non-const) pointer
 * to the bintree_node contained within it
 */
static struct bintree_node * __bintree_node(
    const struct bintree * const bt, const void * p)
{
    return (struct bintree_node *)((uintptr_t)p + bt->off);
}

/*!
 * @private
 *
 * Given a pointer to a node, get a (non-const) pointer
 * to the element containing it
 */
static void * __bintree_element(
    const struct bintree * const bt, const struct bintree_node * const bn)
{
    return (void *)((uintptr_t)bn - bt->off);
}

/*!
 * @private
 *
 * Given a pointer to a node, get a (const) pointer
 * to the element containing it
 */
static const void * bintree_element(
    const struct bintree * const bt, const struct bintree_node * const bn)
{
    return __bintree_element(bt, bn);
}

/*!
 * @private
 *
 * Compare the elements containing the given nodes using the cmp function
 * associated with the tree. This function is called by other objects
 * in the library that are based on the binary tree object
 */
int __bintree_cmp(const struct bintree * const bt,
                  const struct bintree_node * const a,
                  const struct bintree_node * const b)
{
    return bt->cmp.func(bintree_element(bt, a),
                        bintree_element(bt, b),
                        bt->cmp.priv);
}

void bintree_insert(struct bintree * const bt, void * const p)
{
    struct bintree_node * const bn = (void *)((uintptr_t)p + bt->off);
    struct bintree_node ** bp = &bt->root, ** bc = bp;

    while (*bc != NULL) {
        bp = bc;

        if (__bintree_cmp(bt, bn, *bp) < 0) {
            bc = &(*bp)->l;
        } else {
            bc = &(*bp)->r;
        }
    }

    bn->p = *bp;
    bn->l = NULL;
    bn->r = NULL;

    *bc = bn;

    bt->size++;
}

const void * bintree_find(const struct bintree * const bt, const void * f)
{
    const struct bintree_node * const bf = __bintree_node(bt, f);
    struct bintree_node * bn = bt->root;

    while (bn != NULL) {
        const int eq = __bintree_cmp(bt, bf, bn);

        if (eq < 0) {
            bn = bn->l;
        } else if (eq > 0) {
            bn = bn->r;
        } else {
            break;
        }
    }

    if (bn != NULL) {
        return bintree_element(bt, bn);
    }

    return NULL;
}

/*!
 * @private
 *
 * Given a node as a starting point, find the child furthest in
 * the direction indicated by the function pointer @ch
 */
static inline struct bintree_node * bintree_slide(
    const struct bintree_node * bn, __bintree_child_func_t * const ch)
{
    struct bintree_node * c;

    while ((c = *ch((struct bintree_node *)bn)) != NULL) {
            bn = c;
    }

    return (struct bintree_node *)bn;
}

/*!
 * @private
 *
 * Find node in the tree whose element's "value" immediately follows or
 * precedes the given node's element's "value". whether the next or
 * previous node is found is determined by the @l and @r functions.
 *
 * @see __bintree_next
 * @see __bintree_prev
 *
 * inline comments below are for the "prev" case; they'd be reversed
 * for finding the "next"
 */
static struct bintree_node * __bintree_adjacent(
    const struct bintree_node * bn,
    __bintree_child_func_t * const l, __bintree_child_func_t * const r)
{
    struct bintree_node * const c = *l((struct bintree_node *)bn);

    if (c != NULL) {
        /*
         * if the given node has a left (lesser) child,
         * then the greatest value less than the given node,
         * is the left child node's right-most child
         */
        bn = bintree_slide(c, r);
    } else {
        /*
         * since there is no left child, do the reverse
         * of the if clause: move up the tree while the current
         * node is the left child. when it is not the left child
         * (i.e. it is the right child), go up one more time.
         *
         * note that the root may be encountered before the above
         * condition is satisfied, indicating that the desired
         * value is not present in the tree
         */
        while (bn->p != NULL && *l((struct bintree_node *)bn->p) == bn) {
            bn = bn->p;
        }

        bn = bn->p;
    }

    return (struct bintree_node *)bn;
}

/*!
 * @private
 *
 * find the node whose element is the next greater value in the
 * tree than the given node's element
 */
static struct bintree_node * __bintree_next(const struct bintree_node * bn)
{
    return __bintree_adjacent(bn, __bintree_right, __bintree_left);
}

/*!
 * @private
 *
 * find the node whose element is the next lesser value in the
 * tree than the given node's element
 */
static struct bintree_node * __bintree_prev(const struct bintree_node * bn)
{
    return __bintree_adjacent(bn, __bintree_left, __bintree_right);
}

/*!
 * @private
 *
 * this function does the work of actually removing a node from
 * the tree. if the node is a leaf or has only one child: the operation is
 * simple: remove it, putting its child in its place if it had one. if the
 * node has two children, find the next greater element in the tree--it will
 * have one child, at most--remove it (as above) and swap it with the element
 * that was supposed to be removed.
 *
 * returns a pointer to the location in the tree where the desired node
 * was removed from. in the latter case described above, the returned
 * pointer will actually point to a node still in the tree.
 */
const struct bintree_node * __bintree_erase(struct bintree * const bt,
                                            struct bintree_node * const bn)
{
    struct bintree_node * x, * y;

    /*
     * determine which node to remove: the given one if
     * it has 0 or 1 children, otherwise the next greater one
     */
    if (bn->l != NULL && bn->r != NULL) {
        y = __bintree_next(bn);
    } else {
        y = bn;
    }

    /* whichever one it is will/must have 1 child, at most */
    cstl_assert(y->l == NULL || y->r == NULL);

    /* if it had a child, point x at it */
    if (y->l != NULL) {
        x = y->l;
    } else {
        x = y->r;
    }

    /*
     * if it had a child, it's new parent is
     * y's parent (x's former grandparent)
     */
    if (x != NULL) {
        x->p = y->p;
    }

    /*
     * replace y with x as one of y's parent's children
     */
    if (y->p == NULL) {
        bt->root = x;
    } else if (y == y->p->l) {
        y->p->l = x;
    } else {
        y->p->r = x;
    }

    /*
     * at this point, y has been removed from the tree.
     * if y was the desired node, then the work is done.
     * if y was a different node, removed for convenience,
     * y needs to be swapped back into the tree, replacing
     * the node that was supposed to be removed.
     */

    if (y != bn) {
        /* save y's pointers */
        const struct bintree_node t = *y;

        /*
         * make the parent of the node that was supposed to
         * be removed point to y is one of its children instead
         * of the desired node
         */
        if (bn->p == NULL) {
            bt->root = y;
        } else if (bn == bn->p->l) {
            bn->p->l = y;
        } else {
            bn->p->r = y;
        }

        /*
         * modify the children of the node being removed
         * to make y their new parent
         */
        if (bn->l != NULL) {
            bn->l->p = y;
        }
        if (bn->r != NULL) {
            bn->r->p = y;
        }

        /*
         * y adopts all of the pointers belonging to
         * the node being removed, and the node being
         * removed adopts all of y's (saved) pointers
         */
        *y = *bn;
        *bn = t;

        /*
         * it's possible that the (originally) removed node, y,
         * was a direct descendant of bn (the node the caller
         * wanted to remove). in this case, change bn's (formerly
         * y's) parent to be y to more accurately reflect the state
         * of things to the caller
         */
        if (bn->p == bn) {
            bn->p = y;
        }
    }

    bt->size--;

    return y;
}

void * bintree_erase(struct bintree * const bt, const void * const _p)
{
    void * p = (void *)bintree_find(bt, _p);

    if (p != NULL) {
        (void)__bintree_erase(bt, __bintree_node(bt, p));
    }

    return p;
}

/*!
 * @private
 *
 * this operation is used by the red-black tree implementation, but
 * since it operates on the binary tree, independent of any of the
 * red-black additions, it is implemented here. it has the effect of
 * modifying x and x's right child (y) such that y is put in place of
 * x in the tree, x becomes y's left child, and y's (formerly) left
 * child becomes x's new right child. the operation requires that
 * x have a right child prior to calling the function.
 *
 * the operation can be reversed by reversing the @l and @r function
 * pointers
 */
void __bintree_rotate(struct bintree * const bt, struct bintree_node * const x,
                      __bintree_child_func_t * const l,
                      __bintree_child_func_t * const r)
{
    struct bintree_node * const y = *r(x);
    cstl_assert(y != NULL);

    /* y's left child becomes x's right child */
    *r(x) = *l(y);
    if (*l(y) != NULL) {
        (*l(y))->p = x;
    }
    /* y moves into x's position in the tree */
    y->p = x->p;
    if (x->p == NULL) {
        bt->root = y;
    } else if (x == *l(x->p)) {
        *l(x->p) = y;
    } else {
        *r(x->p) = y;
    }
    /* x becomes y's left child */
    *l(y) = x;
    x->p = y;
}

/*!
 * @private
 *
 * this version of the function operates on nodes rather than the tree,
 * so it could be used to treat any given node as the root of the tree
 * and walk the subtree rooted at that node. because of this property,
 * the function also lends itself to being called recursively, which it
 * is. whether the tree is traversed from left-to-right or right-to-left
 * is determined by the @l and @r functions.
 */
static int __bintree_foreach(const struct bintree_node * const _bn,
                             int (* const visit)(const struct bintree_node *,
                                                 bintree_visit_order_t,
                                                 void *),
                             void * const priv,
                             __bintree_child_func_t * const l,
                             __bintree_child_func_t * const r)
{
    struct bintree_node * const bn = (void *)_bn;
    struct bintree_node * const ln = *l(bn), * const rn = *r(bn);
    const int leaf = ln == NULL && rn == NULL;
    int res = 0;

    if (res == 0 && leaf == 0) {
        /* first visit to the current node (if it's a non-leaf) */
        res = visit(bn, BINTREE_VISIT_ORDER_PRE, priv);
    }

    if (res == 0 && ln != NULL) {
        /* visit the subtree rooted at the left child */
        res = __bintree_foreach(ln, visit, priv, l, r);
    }

    if (res == 0) {
        /* visit the current node */
        if (leaf != 0) {
            res = visit(bn, BINTREE_VISIT_ORDER_LEAF, priv);
        } else {
            res = visit(bn, BINTREE_VISIT_ORDER_MID, priv);
        }
    }

    if (res == 0 && rn != NULL) {
        /* visit the subtree rooted at the right child */
        res = __bintree_foreach(rn, visit, priv, l, r);
    }

    if (res == 0 && leaf == 0) {
        /* last visit to the current node (if it's a non-leaf) */
        res = visit(bn, BINTREE_VISIT_ORDER_POST, priv);
    }

    return res;
}

struct bintree_foreach_priv
{
    const struct bintree * bt;
    bintree_const_visit_func_t * visit;
    void * priv;
};

/*!
 * @private
 *
 * the __bintree_foreach function operates on tree nodes. this function
 * translates the node pointer into an element pointer before calling
 * the user-specified function
 */
static int bintree_foreach_visit(const struct bintree_node * const bn,
                                 const bintree_visit_order_t order,
                                 void * const priv)
{
    struct bintree_foreach_priv * const bfp = priv;
    return bfp->visit(bintree_element(bfp->bt, bn), order, bfp->priv);
}

int bintree_foreach(const struct bintree * const bt,
                    bintree_const_visit_func_t * const visit,
                    void * const priv,
                    const bintree_foreach_dir_t dir)
{
    int res = 0;

    if (bt->root != NULL) {
        struct bintree_foreach_priv bfp;

        bfp.bt = bt;
        bfp.visit = visit;
        bfp.priv = priv;

        switch (dir) {
        case BINTREE_FOREACH_DIR_FWD:
            res = __bintree_foreach(bt->root, bintree_foreach_visit, &bfp,
                                    __bintree_left, __bintree_right);
            break;
        case BINTREE_FOREACH_DIR_REV:
            res = __bintree_foreach(bt->root, bintree_foreach_visit, &bfp,
                                    __bintree_right, __bintree_left);
            break;
        }
    }

    return res;
}

void bintree_swap(struct bintree * const a, struct bintree * const b)
{
    struct bintree t;
    cstl_swap(a, b, &t, sizeof(t));
    /*
     * the tree points to the root node, but
     * the parent pointer of the root node
     * is NULL, so no need to do any more
     */
}

struct bintree_clear_priv
{
    struct bintree * bt;
    cstl_clear_func_t * clr;
};

/*!
 * @private
 *
 * call the user specified clear function for each node in the tree.
 * the call is only made for leaf nodes and on the last visit to non-leaf
 * nodes. this ensures that the node isn't visited again after the call
 * which means that the callee can modify the element however it wishes
 * when called.
 */
static int __bintree_clear_visit(const struct bintree_node * const bn,
                                 const bintree_visit_order_t order,
                                 void * const p)
{
    if (order == BINTREE_VISIT_ORDER_POST
        || order == BINTREE_VISIT_ORDER_LEAF) {
        struct bintree_clear_priv * const bcp = p;
        /*
         * explicitly ignore the callee's return value,
         * and proceed through the tree, regardless
         */
        (void)bcp->clr(__bintree_element(bcp->bt, bn), NULL);
    }

    return 0;
}

void bintree_clear(struct bintree * const bt, cstl_clear_func_t * const clr)
{
    if (bt->root != NULL) {
        struct bintree_clear_priv bcp;

        bcp.bt = bt;
        bcp.clr = clr;

        __bintree_foreach(bt->root, __bintree_clear_visit, &bcp,
                          __bintree_left, __bintree_right);

        bt->root  = NULL;
        bt->size = 0;
    }
}

struct bintree_height_priv
{
    size_t min, max;
};

/*!
 * @private
 *
 * whenever a leaf node is encountered, walk up the tree from
 * that node to the root, counting the number of nodes between.
 * store that value if it is the new min and or max height
 * encountered so far
 */
static int __bintree_height(const struct bintree_node * bn,
                            const bintree_visit_order_t order,
                            void * const priv)
{
    struct bintree_height_priv * const hp = priv;

    if (order == BINTREE_VISIT_ORDER_LEAF) {
        size_t h;

        for (h = 0; bn != NULL; h++, bn = bn->p)
            ;

        if (h < hp->min) {
            hp->min = h;
        }

        if (h > hp->max) {
            hp->max = h;
        }
    }

    return 0;
}

void bintree_height(const struct bintree * const bt,
                    size_t * const min, size_t * const max)
{
    struct bintree_height_priv hp;

    hp.min = 0;
    hp.max = 0;

    if (bt->root != NULL) {
        hp.min = SIZE_MAX;
        hp.max = 0;

        __bintree_foreach(bt->root, __bintree_height, &hp,
                          __bintree_left, __bintree_right);
    }

    *min = hp.min;
    *max = hp.max;
}

#ifdef __cfg_test__
#include <check.h>
#include <stdlib.h>

static int __bintree_verify(const struct bintree_node * const bn,
                            const bintree_visit_order_t order,
                            void * const priv)
{
    if (order == BINTREE_VISIT_ORDER_MID
        || order == BINTREE_VISIT_ORDER_LEAF) {
        const struct bintree * const bt = priv;

        if (bn->l != NULL) {
            ck_assert_int_lt(__bintree_cmp(bt, bn->l, bn), 0);
        }
        if (bn->r != NULL) {
            ck_assert_int_ge(__bintree_cmp(bt, bn->r, bn), 0);
        }
    }

    return 0;
}

static void bintree_verify(const struct bintree * const bt)
{
    if (bt->root != NULL) {
        __bintree_foreach(bt->root, __bintree_verify, (void *)bt,
                          __bintree_left, __bintree_right);
    }
}

struct integer {
    int v;
    struct bintree_node bn;
};

static int cmp_integer(const void * const a, const void * const b,
                       void * const p)
{
    (void)p;
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

START_TEST(init)
{
    DECLARE_BINTREE(bt, struct integer, bn, cmp_integer, NULL);
    (void)bt;
}
END_TEST

static void __test_bintree_free(void * const p, void * const x)
{
    (void)x;
    cstl_free(p);
}

static void __test__bintree_fill(struct bintree * const bt, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * const in = cstl_malloc(sizeof(*in));

        do {
            in->v = cstl_rand() % n;
        } while (bintree_find(bt, in) != NULL);

        bintree_insert(bt, in);
        ck_assert_uint_eq(i + 1, bintree_size(bt));

        bintree_verify(bt);
    }
}

static void __test__bintree_drain(struct bintree * const bt)
{
    size_t sz;

    while ((sz = bintree_size(bt)) > 0) {
        struct bintree_node * bn = bt->root;

        __bintree_erase(bt, bn);
        cstl_free((void *)bintree_element(bt, bn));

        ck_assert_uint_eq(sz - 1, bintree_size(bt));

        bintree_verify(bt);
    }

    ck_assert_ptr_null(bt->root);
    ck_assert_uint_eq(bintree_size(bt), 0);
}

START_TEST(fill)
{
    static const size_t n = 100;

    DECLARE_BINTREE(bt, struct integer, bn, cmp_integer, NULL);

    __test__bintree_fill(&bt, n);
    {
        size_t min, max;
        bintree_height(&bt, &min, &max);
    }
    __test__bintree_drain(&bt);
}
END_TEST

static int __test__foreach_fwd_visit(const void * const v,
                                     const bintree_visit_order_t order,
                                     void * const p)
{
    if (order == BINTREE_VISIT_ORDER_MID
        || order == BINTREE_VISIT_ORDER_LEAF) {
        const struct integer * const in = v;
        unsigned int * const i = p;

        ck_assert_uint_eq(*i, in->v);
        (*i)++;
    }

    return 0;
}

START_TEST(walk_fwd)
{
    static const size_t n = 100;

    DECLARE_BINTREE(bt, struct integer, bn, cmp_integer, NULL);
    unsigned int i;

    __test__bintree_fill(&bt, n);

    i = 0;
    bintree_foreach(&bt, __test__foreach_fwd_visit, &i, 0);

    bintree_clear(&bt, __test_bintree_free);
}
END_TEST

static int __test__foreach_rev_visit(const void * const v,
                                     const bintree_visit_order_t order,
                                     void * const p)
{
    if (order == BINTREE_VISIT_ORDER_MID
        || order == BINTREE_VISIT_ORDER_LEAF) {
        const struct integer * const in = v;
        unsigned int * const i = p;

        (*i)--;
        ck_assert_uint_eq(*i, in->v);
    }

    return 0;
}

START_TEST(walk_rev)
{
    static const size_t n = 100;

    DECLARE_BINTREE(bt, struct integer, bn, cmp_integer, NULL);
    unsigned int i;

    __test__bintree_fill(&bt, n);

    i = n;
    bintree_foreach(&bt, __test__foreach_rev_visit, &i, 1);

    bintree_clear(&bt, __test_bintree_free);
}
END_TEST

START_TEST(random_empty)
{
    static const size_t n = 100;

    DECLARE_BINTREE(bt, struct integer, bn, cmp_integer, NULL);

    __test__bintree_fill(&bt, n);

    size_t sz;

    while ((sz = bintree_size(&bt)) > 0) {
        struct integer _in, * in;

        _in.v = cstl_rand() % n;

        in = bintree_erase(&bt, &_in);
        if (in != NULL) {
            cstl_free(in);
            ck_assert_uint_eq(sz - 1, bintree_size(&bt));
        }
    }

    ck_assert_ptr_null(bt.root);
    ck_assert_uint_eq(bt.size, 0);
}
END_TEST

Suite * bintree_suite(void)
{
    Suite * const s = suite_create("bintree");

    TCase * tc;

    tc = tcase_create("bintree");
    tcase_add_test(tc, init);
    tcase_add_test(tc, fill);
    tcase_add_test(tc, walk_fwd);
    tcase_add_test(tc, walk_rev);
    tcase_add_test(tc, random_empty);

    suite_add_tcase(s, tc);

    return s;
}

#endif
