/*!
 * @file
 */

#include "cstl/bintree.h"

#include <assert.h>

/*!
 * @private
 *
 * Given a pointer to an element, return a (non-const) pointer
 * to the cstl_bintree_node contained within it
 */
static struct cstl_bintree_node * __cstl_bintree_node(
    const struct cstl_bintree * const bt, const void * p)
{
    return (struct cstl_bintree_node *)((uintptr_t)p + bt->off);
}

/*!
 * @private
 *
 * Given a pointer to a node, get a (non-const) pointer
 * to the element containing it
 */
static void * __cstl_bintree_element(
    const struct cstl_bintree * const bt,
    const struct cstl_bintree_node * const bn)
{
    return (void *)((uintptr_t)bn - bt->off);
}

/*!
 * @private
 *
 * Given a pointer to a node, get a (const) pointer
 * to the element containing it
 */
static const void * cstl_bintree_element(
    const struct cstl_bintree * const bt,
    const struct cstl_bintree_node * const bn)
{
    return __cstl_bintree_element(bt, bn);
}

/*!
 * @private
 *
 * Compare the elements containing the given nodes using the cmp function
 * associated with the tree. This function is called by other objects
 * in the library that are based on the binary tree object
 */
int __cstl_bintree_cmp(const struct cstl_bintree * const bt,
                       const struct cstl_bintree_node * const a,
                       const struct cstl_bintree_node * const b)
{
    return bt->cmp.func(cstl_bintree_element(bt, a),
                        cstl_bintree_element(bt, b),
                        bt->cmp.priv);
}

void cstl_bintree_insert(struct cstl_bintree * const bt, void * const p)
{
    struct cstl_bintree_node * const bn = (void *)((uintptr_t)p + bt->off);
    struct cstl_bintree_node ** bp = &bt->root, ** bc = bp;

    while (*bc != NULL) {
        bp = bc;

        if (__cstl_bintree_cmp(bt, bn, *bp) < 0) {
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

const void * cstl_bintree_find(const struct cstl_bintree * const bt,
                               const void * f)
{
    const struct cstl_bintree_node * const bf = __cstl_bintree_node(bt, f);
    struct cstl_bintree_node * bn = bt->root;

    while (bn != NULL) {
        const int eq = __cstl_bintree_cmp(bt, bf, bn);

        if (eq < 0) {
            bn = bn->l;
        } else if (eq > 0) {
            bn = bn->r;
        } else {
            break;
        }
    }

    if (bn != NULL) {
        return cstl_bintree_element(bt, bn);
    }

    return NULL;
}

/*!
 * @private
 *
 * Given a node as a starting point, find the child furthest in
 * the direction indicated by the function pointer @ch
 */
static inline struct cstl_bintree_node * cstl_bintree_slide(
    const struct cstl_bintree_node * bn,
    __cstl_bintree_child_func_t * const ch)
{
    struct cstl_bintree_node * c;

    while ((c = *ch((struct cstl_bintree_node *)bn)) != NULL) {
        bn = c;
    }

    return (struct cstl_bintree_node *)bn;
}

/*!
 * @private
 *
 * Find node in the tree whose element's "value" immediately follows or
 * precedes the given node's element's "value". whether the next or
 * previous node is found is determined by the @l and @r functions.
 *
 * @see __cstl_bintree_next
 * @see __cstl_bintree_prev
 *
 * inline comments below are for the "prev" case; they'd be reversed
 * for finding the "next"
 */
static struct cstl_bintree_node * __cstl_bintree_adjacent(
    const struct cstl_bintree_node * bn,
    __cstl_bintree_child_func_t * const l,
    __cstl_bintree_child_func_t * const r)
{
    struct cstl_bintree_node * const c = *l((struct cstl_bintree_node *)bn);

    if (c != NULL) {
        /*
         * if the given node has a left (lesser) child,
         * then the greatest value less than the given node,
         * is the left child node's right-most child
         */
        bn = cstl_bintree_slide(c, r);
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
        while (bn->p != NULL && *l((struct cstl_bintree_node *)bn->p) == bn) {
            bn = bn->p;
        }

        bn = bn->p;
    }

    return (struct cstl_bintree_node *)bn;
}

/*!
 * @private
 *
 * find the node whose element is the next greater value in the
 * tree than the given node's element
 */
static struct cstl_bintree_node * __cstl_bintree_next(
    const struct cstl_bintree_node * bn)
{
    return __cstl_bintree_adjacent(
        bn, __cstl_bintree_right, __cstl_bintree_left);
}

/*!
 * @private
 *
 * find the node whose element is the next lesser value in the
 * tree than the given node's element
 */
static struct cstl_bintree_node * __cstl_bintree_prev(
    const struct cstl_bintree_node * bn)
{
    return __cstl_bintree_adjacent(
        bn, __cstl_bintree_left, __cstl_bintree_right);
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
const struct cstl_bintree_node * __cstl_bintree_erase(
    struct cstl_bintree * const bt,
    struct cstl_bintree_node * const bn)
{
    struct cstl_bintree_node * x, * y;

    /*
     * determine which node to remove: the given one if
     * it has 0 or 1 children, otherwise the next greater one
     */
    if (bn->l != NULL && bn->r != NULL) {
        y = __cstl_bintree_next(bn);
    } else {
        y = bn;
    }

    /* whichever one it is will/must have 1 child, at most */
    assert(y->l == NULL || y->r == NULL);

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
        const struct cstl_bintree_node t = *y;

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

void * cstl_bintree_erase(struct cstl_bintree * const bt,
                          const void * const _p)
{
    void * p = (void *)cstl_bintree_find(bt, _p);

    if (p != NULL) {
        (void)__cstl_bintree_erase(bt, __cstl_bintree_node(bt, p));
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
 * pointers, in which case x must have a left child prior to calling
 * the function
 */
void __cstl_bintree_rotate(struct cstl_bintree * const bt,
                           struct cstl_bintree_node * const x,
                           __cstl_bintree_child_func_t * const l,
                           __cstl_bintree_child_func_t * const r)
{
    struct cstl_bintree_node * const y = *r(x);
    assert(y != NULL);

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
static int __cstl_bintree_foreach(
    const struct cstl_bintree_node * const _bn,
    int (* const visit)(const struct cstl_bintree_node *,
                        cstl_bintree_visit_order_t,
                        void *),
    void * const priv,
    __cstl_bintree_child_func_t * const l,
    __cstl_bintree_child_func_t * const r)
{
    struct cstl_bintree_node * const bn = (void *)_bn;
    struct cstl_bintree_node * const ln = *l(bn), * const rn = *r(bn);
    const int leaf = ln == NULL && rn == NULL;
    int res = 0;

    if (res == 0 && leaf == 0) {
        /* first visit to the current node (if it's a non-leaf) */
        res = visit(bn, CSTL_BINTREE_VISIT_ORDER_PRE, priv);
    }

    if (res == 0 && ln != NULL) {
        /* visit the subtree rooted at the left child */
        res = __cstl_bintree_foreach(ln, visit, priv, l, r);
    }

    if (res == 0) {
        /* visit the current node */
        if (leaf != 0) {
            res = visit(bn, CSTL_BINTREE_VISIT_ORDER_LEAF, priv);
        } else {
            res = visit(bn, CSTL_BINTREE_VISIT_ORDER_MID, priv);
        }
    }

    if (res == 0 && rn != NULL) {
        /* visit the subtree rooted at the right child */
        res = __cstl_bintree_foreach(rn, visit, priv, l, r);
    }

    if (res == 0 && leaf == 0) {
        /* last visit to the current node (if it's a non-leaf) */
        res = visit(bn, CSTL_BINTREE_VISIT_ORDER_POST, priv);
    }

    return res;
}

struct cstl_bintree_foreach_priv
{
    const struct cstl_bintree * bt;
    cstl_bintree_const_visit_func_t * visit;
    void * priv;
};

/*!
 * @private
 *
 * the __cstl_bintree_foreach function operates on tree nodes. this function
 * translates the node pointer into an element pointer before calling
 * the user-specified function
 */
static int cstl_bintree_foreach_visit(
    const struct cstl_bintree_node * const bn,
    const cstl_bintree_visit_order_t order,
    void * const priv)
{
    struct cstl_bintree_foreach_priv * const bfp = priv;
    return bfp->visit(cstl_bintree_element(bfp->bt, bn), order, bfp->priv);
}

int cstl_bintree_foreach(const struct cstl_bintree * const bt,
                         cstl_bintree_const_visit_func_t * const visit,
                         void * const priv,
                         const cstl_bintree_foreach_dir_t dir)
{
    int res = 0;

    if (bt->root != NULL) {
        struct cstl_bintree_foreach_priv bfp;

        bfp.bt = bt;
        bfp.visit = visit;
        bfp.priv = priv;

        switch (dir) {
        case CSTL_BINTREE_FOREACH_DIR_FWD:
            res = __cstl_bintree_foreach(
                bt->root, cstl_bintree_foreach_visit, &bfp,
                __cstl_bintree_left, __cstl_bintree_right);
            break;
        case CSTL_BINTREE_FOREACH_DIR_REV:
            res = __cstl_bintree_foreach(
                bt->root, cstl_bintree_foreach_visit, &bfp,
                __cstl_bintree_right, __cstl_bintree_left);
            break;
        }
    }

    return res;
}

void cstl_bintree_swap(struct cstl_bintree * const a,
                       struct cstl_bintree * const b)
{
    struct cstl_bintree t;
    cstl_swap(a, b, &t, sizeof(t));
    /*
     * the tree points to the root node, but
     * the parent pointer of the root node
     * is NULL, so no need to do any more
     */
}

struct cstl_bintree_clear_priv
{
    struct cstl_bintree * bt;
    cstl_xtor_func_t * clr;
    void * priv;
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
static int __cstl_bintree_clear_visit(
    const struct cstl_bintree_node * const bn,
    const cstl_bintree_visit_order_t order,
    void * const p)
{
    if (order == CSTL_BINTREE_VISIT_ORDER_POST
        || order == CSTL_BINTREE_VISIT_ORDER_LEAF) {
        struct cstl_bintree_clear_priv * const bcp = p;
        /*
         * explicitly ignore the callee's return value,
         * and proceed through the tree, regardless
         */
        (void)bcp->clr(__cstl_bintree_element(bcp->bt, bn), bcp->priv);
    }

    return 0;
}

void cstl_bintree_clear(struct cstl_bintree * const bt,
                        cstl_xtor_func_t * const clr,
                        void * const priv)
{
    if (bt->root != NULL) {
        struct cstl_bintree_clear_priv bcp;

        bcp.bt = bt;
        bcp.clr = clr;
        bcp.priv = priv;

        __cstl_bintree_foreach(bt->root, __cstl_bintree_clear_visit, &bcp,
                               __cstl_bintree_left, __cstl_bintree_right);

        bt->root  = NULL;
        bt->size = 0;
    }
}

struct cstl_bintree_height_priv
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
static int __cstl_bintree_height(const struct cstl_bintree_node * bn,
                                 const cstl_bintree_visit_order_t order,
                                 void * const priv)
{
    struct cstl_bintree_height_priv * const hp = priv;

    if (order == CSTL_BINTREE_VISIT_ORDER_LEAF) {
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

void cstl_bintree_height(const struct cstl_bintree * const bt,
                         size_t * const min, size_t * const max)
{
    struct cstl_bintree_height_priv hp;

    hp.min = 0;
    hp.max = 0;

    if (bt->root != NULL) {
        hp.min = SIZE_MAX;
        hp.max = 0;

        __cstl_bintree_foreach(bt->root, __cstl_bintree_height, &hp,
                               __cstl_bintree_left, __cstl_bintree_right);
    }

    *min = hp.min;
    *max = hp.max;
}

#ifdef __cstl_cfg_test__
// GCOV_EXCL_START
#include <check.h>
#include <stdlib.h>

static int __cstl_bintree_verify(const struct cstl_bintree_node * const bn,
                                 const cstl_bintree_visit_order_t order,
                                 void * const priv)
{
    if (order == CSTL_BINTREE_VISIT_ORDER_MID
        || order == CSTL_BINTREE_VISIT_ORDER_LEAF) {
        const struct cstl_bintree * const bt = priv;

        if (bn->l != NULL) {
            ck_assert_int_lt(__cstl_bintree_cmp(bt, bn->l, bn), 0);
        }
        if (bn->r != NULL) {
            ck_assert_int_ge(__cstl_bintree_cmp(bt, bn->r, bn), 0);
        }
    }

    return 0;
}

static void cstl_bintree_verify(const struct cstl_bintree * const bt)
{
    if (bt->root != NULL) {
        __cstl_bintree_foreach(bt->root, __cstl_bintree_verify, (void *)bt,
                               __cstl_bintree_left, __cstl_bintree_right);
    }
}

struct integer {
    int v;
    struct cstl_bintree_node bn;
};

static int cmp_integer(const void * const a, const void * const b,
                       void * const p)
{
    (void)p;
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

START_TEST(init)
{
    DECLARE_CSTL_BINTREE(bt, struct integer, bn, cmp_integer, NULL);
    (void)bt;
}
END_TEST

static void __test_cstl_bintree_free(void * const p, void * const x)
{
    (void)x;
    free(p);
}

static void __test__cstl_bintree_fill(struct cstl_bintree * const bt,
                                      const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * const in = malloc(sizeof(*in));

        do {
            in->v = rand() % n;
        } while (cstl_bintree_find(bt, in) != NULL);

        cstl_bintree_insert(bt, in);
        ck_assert_uint_eq(i + 1, cstl_bintree_size(bt));

        cstl_bintree_verify(bt);
    }
}

static void __test__cstl_bintree_drain(struct cstl_bintree * const bt)
{
    size_t sz;

    while ((sz = cstl_bintree_size(bt)) > 0) {
        struct cstl_bintree_node * bn = bt->root;

        __cstl_bintree_erase(bt, bn);
        free((void *)cstl_bintree_element(bt, bn));

        ck_assert_uint_eq(sz - 1, cstl_bintree_size(bt));

        cstl_bintree_verify(bt);
    }

    ck_assert_ptr_null(bt->root);
    ck_assert_uint_eq(cstl_bintree_size(bt), 0);
}

START_TEST(fill)
{
    static const size_t n = 100;

    DECLARE_CSTL_BINTREE(bt, struct integer, bn, cmp_integer, NULL);

    __test__cstl_bintree_fill(&bt, n);
    {
        size_t min, max;
        cstl_bintree_height(&bt, &min, &max);
    }
    __test__cstl_bintree_drain(&bt);
}
END_TEST

static int __test__foreach_fwd_visit(const void * const v,
                                     const cstl_bintree_visit_order_t order,
                                     void * const p)
{
    if (order == CSTL_BINTREE_VISIT_ORDER_MID
        || order == CSTL_BINTREE_VISIT_ORDER_LEAF) {
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

    DECLARE_CSTL_BINTREE(bt, struct integer, bn, cmp_integer, NULL);
    struct cstl_bintree_node * node;
    unsigned int i;

    __test__cstl_bintree_fill(&bt, n);

    i = 0;
    cstl_bintree_foreach(&bt,
                         __test__foreach_fwd_visit, &i,
                         CSTL_BINTREE_FOREACH_DIR_FWD);

    node = cstl_bintree_slide(bt.root, __cstl_bintree_left);
    ck_assert_ptr_nonnull(node);
    i = 0;
    while (node != NULL) {
        const struct integer * const in = __cstl_bintree_element(&bt, node);
        ck_assert_uint_eq(i, in->v);
        i++;
        node = __cstl_bintree_next(node);
    }

    cstl_bintree_clear(&bt, __test_cstl_bintree_free, NULL);
}
END_TEST

static int __test__foreach_rev_visit(const void * const v,
                                     const cstl_bintree_visit_order_t order,
                                     void * const p)
{
    if (order == CSTL_BINTREE_VISIT_ORDER_MID
        || order == CSTL_BINTREE_VISIT_ORDER_LEAF) {
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

    DECLARE_CSTL_BINTREE(bt, struct integer, bn, cmp_integer, NULL);
    struct cstl_bintree_node * node;
    unsigned int i;

    __test__cstl_bintree_fill(&bt, n);

    i = n;
    cstl_bintree_foreach(&bt,
                         __test__foreach_rev_visit, &i,
                         CSTL_BINTREE_FOREACH_DIR_REV);

    node = cstl_bintree_slide(bt.root, __cstl_bintree_right);
    ck_assert_ptr_nonnull(node);
    i = n;
    while (node != NULL) {
        const struct integer * const in = __cstl_bintree_element(&bt, node);
        i--;
        ck_assert_uint_eq(i, in->v);
        node = __cstl_bintree_prev(node);
    }

    cstl_bintree_clear(&bt, __test_cstl_bintree_free, NULL);
}
END_TEST

START_TEST(random_empty)
{
    static const size_t n = 100;

    DECLARE_CSTL_BINTREE(bt, struct integer, bn, cmp_integer, NULL);
    size_t sz;

    __test__cstl_bintree_fill(&bt, n);

    while ((sz = cstl_bintree_size(&bt)) > 0) {
        struct integer _in, * in;

        _in.v = rand() % n;

        in = cstl_bintree_erase(&bt, &_in);
        if (in != NULL) {
            free(in);
            ck_assert_uint_eq(sz - 1, cstl_bintree_size(&bt));
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

// GCOV_EXCL_STOP
#endif
