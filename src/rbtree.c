/*!
 * @file
 */

#include "cstl/rbtree.h"

#include <assert.h>

/*! @private */
static inline struct cstl_rbtree_node * __cstl_rbtree_node(
    const struct cstl_rbtree * const t, const void * e)
{
    return (void *)((uintptr_t)e + t->off);
}

/*!
 * @private
 *
 * Given a pointer to a binary tree node,
 * get a pointer to the red-black tree node's color
 */
static inline cstl_rbtree_color_t * BN_COLOR(
    const struct cstl_bintree_node * const bn)
{
    return &((struct cstl_rbtree_node *)(
                 (uintptr_t)bn - offsetof(struct cstl_rbtree_node, n)))->c;
}

/*!
 * @private
 *
 * this function is called as a result of x and x's parent
 * both being red. the goal of this function is to push that
 * property violation up the tree, toward the root without
 * breaking the "same number of black nodes on every path" rule
 *
 * x's parent is assumed to be the left child of x's grandparent
 * upon entry to this function. for the case where it is the right
 * child, the @l and @r parameters must be reversed
 */
static struct cstl_bintree_node * cstl_rbtree_fix_insertion(
    struct cstl_bintree * const t, struct cstl_bintree_node * x,
    __cstl_bintree_child_func_t * const l,
    __cstl_bintree_child_func_t * const r)
{
    struct cstl_bintree_node * const y = *r(x->p->p);

    /*
     * if the tree is not violating any of the red-black
     * properties aside from x and it's parent both being
     * red, then x's grandparent is guaranteed to be black.
     */

    if (y != NULL && *BN_COLOR(y) == CSTL_RBTREE_COLOR_R) {
        /*
         * if x's parent's sibling is also red, then
         * the parent and the sibling can be changed to
         * black and the grandparent to red. now the
         * red-red violation (if one exists) is between
         * x's grandparent and great grandparent
         */
        *BN_COLOR(x->p) = CSTL_RBTREE_COLOR_B;
        *BN_COLOR(y) = CSTL_RBTREE_COLOR_B;
        *BN_COLOR(x->p->p) = CSTL_RBTREE_COLOR_R;
        x = x->p->p;
    } else {
        if (x == *r(x->p)) {
            /*
             * x is the right child. rotate such that
             * x's parent becomes x's left child and
             * x becomes x's grandparent's left child.
             *
             * note that x is moved down to point at
             * its former parent (now its left child)
             */
            x = x->p;
            __cstl_bintree_rotate(t, x, l, r);
        }

        /*
         * x is now a left child.
         *
         * x's grandparent is a black node whose left child
         * and left child's child are both red. rotate the
         * tree right about the grandparent and re-color
         * the nodes to make the position formerly occupied
         * by the grandparent black with two red children
         */

        *BN_COLOR(x->p) = CSTL_RBTREE_COLOR_B;
        *BN_COLOR(x->p->p) = CSTL_RBTREE_COLOR_R;
        __cstl_bintree_rotate(t, x->p->p, r, l);
    }

    return x;
}

void cstl_rbtree_insert(struct cstl_rbtree * const t, void * const p)
{
    struct cstl_rbtree_node * const n = __cstl_rbtree_node(t, p);
    struct cstl_bintree_node * x;

    /*
     * insert as normal, with the new node colored
     */
    cstl_bintree_insert(&t->t, p);
    n->c = CSTL_RBTREE_COLOR_R;

    /*
     * it's possible that the new node's parent is red,
     * which is a violation of the "red nodes can only
     * have black children" property
     */
    x = &n->n;
    while (x->p != NULL && *BN_COLOR(x->p) == CSTL_RBTREE_COLOR_R) {
        /*
         * if has a parent (i.e. is not the root) and is
         * red, then x must have a grandparent because
         * the root is always black
         */

        if (x->p == x->p->p->l) {
            x = cstl_rbtree_fix_insertion(
                    &t->t, x,
                    __cstl_bintree_left, __cstl_bintree_right);
        } else {
            x = cstl_rbtree_fix_insertion(
                    &t->t, x,
                    __cstl_bintree_right, __cstl_bintree_left);
        }
    }

    *BN_COLOR(t->t.root) = CSTL_RBTREE_COLOR_B;
}

/*!
 * @private
 *
 * x must be black and not be the root upon entry to the function.
 *
 * the function is written as if x is its parent's left child, but
 * the code can operate in the case where x is the right child by
 * reversing the left/right functions passed as the @l and @r
 * parameters
 */
static struct cstl_bintree_node * cstl_rbtree_fix_deletion(
    struct cstl_bintree * const t, struct cstl_bintree_node * x,
    __cstl_bintree_child_func_t * const l,
    __cstl_bintree_child_func_t * const r)
{
    struct cstl_bintree_node * w;

    /*
     * the reason this function gets called is because
     * there is 1 fewer black node on the path to x than
     * every other path in the tree. for that reason,
     * x's (w) sibling must be non-NULL, otherwise, the path
     * to the sibling would have the same number of blacks
     * as the path to x.
     */
    w = *r(x->p);

    if (*BN_COLOR(w) == CSTL_RBTREE_COLOR_R) {
        /*
         * if the sibling is red, it must have black children.
         *
         * the tree is rotated left which makes w x's grandparent
         * and w's (left child), x's new sibling. colors are adjusted
         * to maintain the current red-black status quo. one of
         * the conditions below now applies
         */
        *BN_COLOR(w) = CSTL_RBTREE_COLOR_B;
        *BN_COLOR(x->p) = CSTL_RBTREE_COLOR_R;
        __cstl_bintree_rotate(t, x->p, l, r);
        w = *r(x->p);
    }

    /*
     * based on the case above, x's sibling was either black
     * or it was transformed so that x's sibling is now black.
     */
    if ((*l(w) == NULL || *BN_COLOR(*l(w)) == CSTL_RBTREE_COLOR_B)
        && (*r(w) == NULL || *BN_COLOR(*r(w)) == CSTL_RBTREE_COLOR_B)) {
        /* if w has two black children, then make it red */
        *BN_COLOR(w) = CSTL_RBTREE_COLOR_R;
        /*
         * the tree is good up to this point,
         * move x further up the tree
         */
        x = x->p;
    } else {
        /* else w has at least one red child */
        if (*r(w) == NULL || *BN_COLOR(*r(w)) == CSTL_RBTREE_COLOR_B) {
            /*
             * if w's right child is black, then the left has
             * to be red. the case looks something like the first
             * if clause above. the colors are adjusted similarly
             * and the tree rotated. this maintains the status quo
             * as before and x's new sibling (w) has a red right
             * child
             */
            *BN_COLOR(*l(w)) = CSTL_RBTREE_COLOR_B;
            *BN_COLOR(w) = CSTL_RBTREE_COLOR_R;
            __cstl_bintree_rotate(t, w, r, l);
            w = *r(x->p);
        }

        /*
         * either the right child of w is red (and the if clause
         * above was skipped) or the if clause was entered which
         * transformed the tree such that the right child of w
         * is *now* red
         *
         * rotating the tree left allows the nodes to the left
         * of w (after the rotation) to be colored black such
         * that the missing black caused by the deletion can
         * be restored
         */

        *BN_COLOR(w) = *BN_COLOR(x->p);
        *BN_COLOR(x->p) = CSTL_RBTREE_COLOR_B;
        *BN_COLOR(*r(w)) = CSTL_RBTREE_COLOR_B;
        __cstl_bintree_rotate(t, x->p, l, r);

        /*
         * setting x to be the root tells the caller to
         * stop fixing since there is no further up the
         * tree to move
         */
        x = t->root;
    }

    return x;
}

/*! @private */
void __cstl_rbtree_erase(struct cstl_rbtree * const t,
                         struct cstl_rbtree_node * const n)
{
    const struct cstl_bintree_node * const y =
        __cstl_bintree_erase(&t->t, &n->n);

    /*
     * y points to the location in the tree from where the
     * node was *supposed* to be removed. the line below
     * captures the color that was *supposed* to be removed
     * from the tree.
     */
    const cstl_rbtree_color_t c = *BN_COLOR(y);
    /*
     * restore the correct color to the node that remains
     * in the tree. (note that if the node that was *supposed*
     * to be removed *was* removed, then this line has no
     * effect because y == n
     */
    *BN_COLOR(y) = n->c;

    /*
     * if the color of the removed node was black, it's
     * 1.) possible that the rule that a red node must have
     * black children, and 2.) certain that the rule that
     * the same number of black nodes be on every path from
     * the root to the leaves have been violated, and more
     * work is needed to fix that.
     */
    if (c == CSTL_RBTREE_COLOR_B) {
        struct cstl_rbtree_node _x;
        struct cstl_bintree_node * x;

        /*
         * the removed node can only have had 0 or 1
         * children; see __cstl_bintree_erase for an explanation
         */
        assert(n->n.l == NULL || n->n.r == NULL);

        /*
         * point x at one of the removed node's (former)
         * children. if it had no children, fake one,
         * whose color is black, by convention.
         */
        if (n->n.l != NULL) {
            x = n->n.l;
        } else if (n->n.r != NULL) {
            x = n->n.r;
        } else {
            x = &_x.n;

            x->p = n->n.p;
            *BN_COLOR(x) = CSTL_RBTREE_COLOR_B;
        }

        /*
         * x's parent is the removed node's parent,
         * x's former grandparent
         */
        assert(x->p == n->n.p);

        /*
         * any path to x has 1 too few black nodes in
         * its path since its parent, a black node, was
         * removed from the tree. work up the tree, trying
         * to restore red-black properties
         */

        while (x->p != NULL && *BN_COLOR(x) == CSTL_RBTREE_COLOR_B) {
            if (x == x->p->l || (x == &_x.n && x->p->l == NULL)) {
                x = cstl_rbtree_fix_deletion(
                        &t->t, x,
                        __cstl_bintree_left, __cstl_bintree_right);
            } else {
                x = cstl_rbtree_fix_deletion(
                        &t->t, x,
                        __cstl_bintree_right, __cstl_bintree_left);
            }
        }

        /*
         * if the loop encounters a red node, making it
         * black fixes the number of black nodes on
         * the path to x
         */
        *BN_COLOR(x) = CSTL_RBTREE_COLOR_B;
    }
}

void * cstl_rbtree_erase(struct cstl_rbtree * const t, const void * const _p)
{
    void * const p = (void *)cstl_rbtree_find(t, _p);
    if (p != NULL) {
        __cstl_rbtree_erase(t, __cstl_rbtree_node(t, p));
    }
    return p;
}

#ifdef __cstl_cfg_test__
// GCOV_EXCL_START
#include <check.h>
#include <stdlib.h>

struct integer
{
    int v;
    struct cstl_rbtree_node n;
};

static int cmp_integer(const void * const a, const void * const b,
                       void * const p)
{
    (void)p;
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

START_TEST(init)
{
    DECLARE_CSTL_RBTREE(t, struct integer, n, cmp_integer, NULL);
    (void)t;
}
END_TEST

static int __cstl_rbtree_verify(const void * const elem,
                                const cstl_bintree_visit_order_t order,
                                void * const priv)
{
    if (order == CSTL_BINTREE_VISIT_ORDER_MID
        || order == CSTL_BINTREE_VISIT_ORDER_LEAF) {
        const struct cstl_bintree * const t = priv;
        const struct cstl_bintree_node * const bn =
            &((const struct integer *)elem)->n.n;

        size_t bh = 0;

        if (*BN_COLOR(bn) == CSTL_RBTREE_COLOR_R) {
            ck_assert(bn->l == NULL
                      || *BN_COLOR(bn->l) == CSTL_RBTREE_COLOR_B);
            ck_assert(bn->r == NULL
                      || *BN_COLOR(bn->r) == CSTL_RBTREE_COLOR_B);
        }

        if (bn->l != NULL) {
            ck_assert_int_lt(__cstl_bintree_cmp(t, bn->l, bn), 0);
        }
        if (bn->r != NULL) {
            ck_assert_int_ge(__cstl_bintree_cmp(t, bn->r, bn), 0);
        }

        if (bn->l == NULL && bn->r == NULL) {
            const struct cstl_bintree_node * n;
            size_t h;

            for (h = 0, n = bn; n != NULL; n = n->p) {
                if (*BN_COLOR(n) == CSTL_RBTREE_COLOR_B) {
                    h++;
                }
            }

            ck_assert(bh == 0 || h == bh);
            bh = h;
        }
    }

    return 0;
}

static void cstl_rbtree_verify(const struct cstl_rbtree * const t)
{
    if (t->t.root != NULL) {
        size_t min, max;

        cstl_rbtree_height(t, &min, &max);
        ck_assert_uint_le(max, 2 * log2(cstl_rbtree_size(t) + 1));
        ck_assert_uint_le(max, 2 * min);

        cstl_rbtree_foreach(
            t, __cstl_rbtree_verify, (void *)&t->t,
            CSTL_BINTREE_FOREACH_DIR_FWD);
    }
}

static void __test_cstl_rbtree_free(void * const p, void * const x)
{
    (void)x;
    free(p);
}

static void __test__cstl_rbtree_fill(struct cstl_rbtree * const t,
                                     const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * const in = malloc(sizeof(*in));
        in->v = i;

        cstl_rbtree_insert(t, in);
        ck_assert_uint_eq(i + 1, cstl_rbtree_size(t));
    }
}

START_TEST(fill)
{
    static const size_t n = 100;

    DECLARE_CSTL_RBTREE(t, struct integer, n, cmp_integer, NULL);

    __test__cstl_rbtree_fill(&t, n);
    cstl_rbtree_verify(&t);
    cstl_rbtree_clear(&t, __test_cstl_rbtree_free, NULL);
}
END_TEST

START_TEST(random_fill)
{
    static const size_t n = 100;

    DECLARE_CSTL_RBTREE(t, struct integer, n, cmp_integer, NULL);
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * const in = malloc(sizeof(*in));

        do {
            in->v = rand() % n;
        } while (cstl_rbtree_find(&t, in) != NULL);

        cstl_rbtree_insert(&t, in);
        ck_assert_uint_eq(i + 1, cstl_rbtree_size(&t));
    }

    cstl_rbtree_verify(&t);
    cstl_rbtree_clear(&t, __test_cstl_rbtree_free, NULL);
}
END_TEST

START_TEST(random_empty)
{
    static const size_t n = 100;

    DECLARE_CSTL_RBTREE(t, struct integer, n, cmp_integer, NULL);
    size_t sz;

    __test__cstl_rbtree_fill(&t, n);

    while ((sz = cstl_rbtree_size(&t)) > 0) {
        struct integer _in, * in;

        _in.v = rand() % n;

        in = cstl_rbtree_erase(&t, &_in);
        if (in != NULL) {
            free(in);
            ck_assert_uint_eq(sz - 1, cstl_rbtree_size(&t));

            cstl_rbtree_verify(&t);
        }
    }

    ck_assert_ptr_null(t.t.root);
    ck_assert_uint_eq(cstl_rbtree_size(&t), 0);
}
END_TEST

Suite * rbtree_suite(void)
{
    Suite * const s = suite_create("rbtree");

    TCase * tc;

    tc = tcase_create("rbtree");
    tcase_add_test(tc, init);
    tcase_add_test(tc, fill);
    tcase_add_test(tc, random_fill);
    tcase_add_test(tc, random_empty);

    suite_add_tcase(s, tc);

    return s;
}

// GCOV_EXCL_STOP
#endif
