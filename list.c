/*!
 * @file
 */

#include "list.h"

/*! @private */
static void * __list_element(const struct list * const l,
                             const struct list_node * const n)
{
    return (void *)((uintptr_t)n - l->off);
}

/*! @private */
static struct list_node * __list_node(const struct list * const l,
                                      const void * const e)
{
    return (void *)((uintptr_t)e + l->off);
}

/*! @private */
static struct list_node ** __list_next(struct list_node * const n)
{
    return &n->n;
}

/*! @private */
static struct list_node ** __list_prev(struct list_node * const n)
{
    return &n->p;
}

/*! @private */
static void __list_insert(struct list * const l,
                          struct list_node * const p,
                          struct list_node * const n)
{
    n->n = p->n;
    n->p = p;

    /* node after p/n points back to n */
    n->n->p = n;
    /* p points to n as its next */
    p->n = n;

    l->size++;
}

void list_insert(struct list * const l, void * const pe, void * const e)
{
    __list_insert(l, __list_node(l, pe), __list_node(l, e));
}

/*! @private */
static void * __list_erase(struct list * const l,
                           struct list_node * const n)
{
    n->n->p = n->p;
    n->p->n = n->n;

    l->size--;

    return __list_element(l, n);
}

void list_erase(struct list * const l, void * const e)
{
    __list_erase(l, __list_node(l, e));
}

void * list_front(struct list * const l)
{
    if (l->size > 0) {
        return __list_element(l, l->h.n);
    }
    return NULL;
}

void * list_back(struct list * const l)
{
    if (l->size > 0) {
        return __list_element(l, l->h.p);
    }
    return NULL;
}

void list_push_front(struct list * const l, void * const e)
{
    __list_insert(l, &l->h, __list_node(l, e));
}

void list_push_back(struct list * const l, void * const e)
{
    __list_insert(l, l->h.p, __list_node(l, e));
}

void * list_pop_front(struct list * const l)
{
    if (l->size > 0) {
        return __list_erase(l, l->h.n);
    }
    return NULL;
}

void * list_pop_back(struct list * const l)
{
    if (l->size > 0) {
        return __list_erase(l, l->h.p);
    }
    return NULL;
}

int list_foreach(struct list * const l,
                 cstl_visit_func_t * const visit, void * const p,
                 const list_foreach_dir_t dir)
{
    struct list_node ** (* next)(struct list_node *);
    struct list_node * c, * n;
    int res = 0;

    switch (dir) {
    default:
    case LIST_FOREACH_DIR_FWD:
        next = __list_next;
        break;
    case LIST_FOREACH_DIR_REV:
        next = __list_prev;
        break;
    }

    for (c = *next(&l->h), n = *next(c);
         res == 0 && c != &l->h;
         c = n, n = *next(c)) {
        res = visit(__list_element(l, c), p);
    }

    return res;
}

/*! @private */
struct list_find_priv
{
    cstl_compare_func_t * cmp;
    void * p;
    const void * e;
};

/*! @private */
static int list_find_visit(void * const e, void * const p)
{
    struct list_find_priv * lfp = p;

    if (lfp->cmp(lfp->e, e, lfp->p) == 0) {
        lfp->e = e;
        return 1;
    }

    return 0;
}

void * list_find(const struct list * const l, const void * const e,
                 cstl_compare_func_t * const cmp, void * const cmp_p,
                 const list_foreach_dir_t dir)
{
    struct list_find_priv lfp;

    lfp.cmp = cmp;
    lfp.p   = cmp_p;
    lfp.e   = e;

    if (list_foreach((struct list *)l, list_find_visit, &lfp, dir) > 0) {
        return (void *)lfp.e;
    }

    return NULL;
}

void list_swap(struct list * const a, struct list * const b)
{
    struct list t;

    cstl_swap(a, b, &t, sizeof(t));

#ifndef NO_DOC
#define LIST_SWAP_FIX(L)                        \
    do {                                        \
        if (L->size == 0) {                     \
            L->h.n = L->h.p = &L->h;            \
        } else {                                \
            L->h.n->p = L->h.p->n = &L->h;      \
        }                                       \
    } while (0)
#endif

    LIST_SWAP_FIX(a);
    LIST_SWAP_FIX(b);

#undef LIST_SWAP_FIX
}

void list_clear(struct list * const l, cstl_clear_func_t * const clr)
{
    while (l->size > 0) {
        clr(__list_erase(l, l->h.n), NULL);
    }
}

/*
 * time complexity is linear. the number of
 * loops/operations required is n/2, though.
 */
void list_reverse(struct list * const l)
{
    struct list_node * i, * j, * k;

    /*
     * i points to the first item in the list; j points
     * to the last. the objects at i and j are swapped,
     * and then i and j are advanced toward the center.
     * the loop terminates when i and j point to the same
     * or adjacent (with i still before j) nodes.
     *
     * k is used as a temporary variable
     */
    for (i = l->h.n, j = l->h.p;
         i != j && i->n != j;
         k = i->p, i = j->n, j = k) {
        struct list_node t;

        /* point the nodes surrounding i at j... */
        i->p->n = j;
        i->n->p = j;

        /* ...and the nodes surrounding j at i */
        j->n->p = i;
        j->p->n = i;

        /* then swap i and j */
        cstl_swap(i, j, &t, sizeof(t));
    }

    /*
     * if i and j point to the same node, no need to
     * swap them. if they're different, they need to
     * be swapped, but special handling is required
     */

    if (i->n == j) {
        /*
         * point the nodes before i and after j
         * at j and i, respectively
         */
        i->p->n = j;
        j->n->p = i;

        /* then fix up i and j's pointers */
        i->n = j->n;
        j->n = i;

        j->p = i->p;
        i->p = j;
    }
}

void list_concat(struct list * const d, struct list * const s)
{
    if (d != s && d->off == s->off && s->size > 0) {
        /* beginning of s points back at end of d */
        s->h.n->p = d->h.p;
        /* end of s points at head of d */
        s->h.p->n = &d->h;

        /* end of d points at beginning of s */
        d->h.p->n = s->h.n;
        /* head of d points back at end of s */
        d->h.p = s->h.p;

        d->size += s->size;

        /* leave the original list in a usable state */
        list_init(s, s->off);
    }
}

void list_sort(struct list * const l,
               cstl_compare_func_t * const cmp, void * const priv)
{
    /* no need to sort a list with a size of one */

    if (l->size > 1) {
        struct list _l[2];
        struct list_node * t;

        /*
         * break the list l into two halves,
         * into _l[0] and _l[1]
         */

        list_init(&_l[0], l->off);
        list_init(&_l[1], l->off);

        /* find the middle of the list */
        for (t = &l->h;
             _l[0].size < l->size / 2;
             t = t->n, _l[0].size++)
            ;

        _l[0].h.n = l->h.n;
        _l[0].h.p = t;

        _l[1].h.n = t->n;
        _l[1].h.p = l->h.p;

        _l[0].h.n->p = _l[0].h.p->n = &_l[0].h;
        _l[1].h.n->p = _l[1].h.p->n = &_l[1].h;

        _l[1].size = l->size - _l[0].size;
        list_init(l, l->off);

        /* sort the two halves */
        list_sort(&_l[0], cmp, priv);
        list_sort(&_l[1], cmp, priv);

        /*
         * merge the two sorted halves back together by
         * comparing the nodes at the front of each list
         * and moving the smaller one to the output
         */
        while (_l[0].size > 0 && _l[1].size > 0) {
            struct list_node * n;
            struct list * ol;

            if (cmp(__list_element(&_l[0], _l[0].h.n),
                    __list_element(&_l[1], _l[1].h.n),
                    priv) <= 0) {
                ol = &_l[0];
            } else {
                ol = &_l[1];
            }

            n = ol->h.n;
            __list_erase(ol, n);
            __list_insert(l, l->h.p, n);
        }

        /* append whatever is left to the output */
        if (_l[0].size > 0) {
            list_concat(l, &_l[0]);
        } else {
            list_concat(l, &_l[1]);
        }
    }
}

#ifdef __cfg_test__
#include <check.h>
#include <stdlib.h>

struct integer {
    int v;
    struct list_node ln;
};

static int cmp_integer(const void * const a, const void * const b,
                       void * const p)
{
    (void)p;
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

static void __test_list_free(void * const p, void * const x)
{
    (void)x;
    cstl_free(p);
}

static void __test__list_fill(struct list * l, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * in = cstl_malloc(sizeof(*in));

        in->v = cstl_rand() % n;
        list_push_back(l, in);
    }

    ck_assert_uint_eq(n, list_size(l));
}

START_TEST(fill)
{
    static const size_t n = 100;
    DECLARE_LIST(l, struct integer, ln);

    __test__list_fill(&l, n);

    list_clear(&l, __test_list_free);
    ck_assert_uint_eq(list_size(&l), 0);
}
END_TEST

START_TEST(concat)
{
    static const size_t n = 4;
    DECLARE_LIST(l1, struct integer, ln);
    DECLARE_LIST(l2, struct integer, ln);

    __test__list_fill(&l1, n);
    __test__list_fill(&l2, n);

    list_concat(&l1, &l2);
    ck_assert_uint_eq(list_size(&l1), 2 * n);
    ck_assert_uint_eq(list_size(&l2), 0);

    list_clear(&l1, __test_list_free);
    list_clear(&l2, __test_list_free);
}
END_TEST

static int list_verify_sorted(void * const e, void * const p)
{
    struct integer ** in = p;

    if (*in != NULL) {
        ck_assert_int_ge(((struct integer *)e)->v, (*in)->v);
    }

    *in = e;

    return 0;
}

START_TEST(sort)
{
    static const size_t n = 100;
    DECLARE_LIST(l, struct integer, ln);

    struct integer * in = NULL;

    __test__list_fill(&l, n);

    list_sort(&l, cmp_integer, NULL);
    ck_assert_uint_eq(n, list_size(&l));
    list_foreach(&l, list_verify_sorted, &in, LIST_FOREACH_DIR_FWD);

    list_clear(&l, __test_list_free);
    ck_assert_uint_eq(list_size(&l), 0);
}
END_TEST

static int list_verify_sorted_rev(void * const e, void * const p)
{
    struct integer ** in = p;

    if (*in != NULL) {
        ck_assert_int_le(((struct integer *)e)->v, (*in)->v);
    }

    *in = e;

    return 0;
}

START_TEST(reverse)
{
    static const size_t n = 100;
    DECLARE_LIST(l, struct integer, ln);

    struct integer * in = NULL;

    __test__list_fill(&l, n);

    list_sort(&l, cmp_integer, NULL);
    list_reverse(&l);
    list_foreach(&l, list_verify_sorted_rev, &in, LIST_FOREACH_DIR_FWD);

    list_clear(&l, __test_list_free);
    ck_assert_uint_eq(list_size(&l), 0);
}
END_TEST

START_TEST(swap)
{
    DECLARE_LIST(l1, struct integer, ln);
    DECLARE_LIST(l2, struct integer, ln);

    __test__list_fill(&l1, 0);
    list_swap(&l1, &l2);
    ck_assert_int_eq(list_size(&l1), 0);
    ck_assert_int_eq(list_size(&l2), 0);
    list_clear(&l1, __test_list_free);
    list_clear(&l2, __test_list_free);

    __test__list_fill(&l1, 1);
    list_swap(&l1, &l2);
    ck_assert_int_eq(list_size(&l1), 0);
    ck_assert_int_eq(list_size(&l2), 1);
    list_clear(&l1, __test_list_free);
    list_clear(&l2, __test_list_free);

    __test__list_fill(&l1, 2);
    list_swap(&l1, &l2);
    ck_assert_int_eq(list_size(&l1), 0);
    ck_assert_int_eq(list_size(&l2), 2);
    list_clear(&l1, __test_list_free);
    list_clear(&l2, __test_list_free);

    __test__list_fill(&l1, 2);
    __test__list_fill(&l2, 3);
    list_swap(&l1, &l2);
    ck_assert_int_eq(list_size(&l1), 3);
    ck_assert_int_eq(list_size(&l2), 2);
    list_clear(&l1, __test_list_free);
    list_clear(&l2, __test_list_free);
}
END_TEST

Suite * list_suite(void)
{
    Suite * const s = suite_create("list");

    TCase * tc;

    tc = tcase_create("list");
    tcase_add_test(tc, fill);
    tcase_add_test(tc, concat);
    tcase_add_test(tc, sort);
    tcase_add_test(tc, reverse);
    tcase_add_test(tc, swap);
    suite_add_tcase(s, tc);

    return s;
}

#endif
