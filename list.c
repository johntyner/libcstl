#include "list.h"

static void * __list_element(const struct list * const l,
                             const struct list_node * const n)
{
    return (void *)((uintptr_t)n - l->off);
}

static struct list_node * __list_node(const struct list * const l,
                                      const void * const e)
{
    return (void *)((uintptr_t)e + l->off);
}

static struct list_node ** __list_next(struct list_node * const n)
{
    return &n->n;
}

static struct list_node ** __list_prev(struct list_node * const n)
{
    return &n->p;
}

static void __list_insert(struct list * const l,
                          struct list_node * const p,
                          struct list_node * const n)
{
    n->n = p->n;
    n->p = p;

    p->n->p = n;
    p->n = n;

    l->size++;
}

void list_insert(struct list * const l, void * const pe, void * const e)
{
    __list_insert(l, __list_node(l, pe), __list_node(l, e));
}

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
    void * e = NULL;
    if (l->size > 0) {
        e = __list_erase(l, l->h.n);
    }
    return e;
}

void * list_pop_back(struct list * const l)
{
    void * e = NULL;
    if (l->size > 0) {
        e = __list_erase(l, l->h.p);
    }
    return e;
}

static int __list_foreach(
    struct list * const l,
    cstl_visit_func_t * const visit, void * const p,
    struct list_node ** (* const next)(struct list_node *))
{
    struct list_node * c, * n;
    int res = 0;

    for (c = *next(&l->h), n = *next(c);
         res == 0 && c != &l->h;
         c = n, n = *next(c)) {
        res = visit(__list_element(l, c), p);
    }

    return res;
}


int list_foreach(struct list * const l,
                 cstl_visit_func_t * const visit, void * const p,
                 const list_foreach_dir_t dir)
{
    int res = 0;

    switch (dir) {
    case LIST_WALK_DIR_FWD:
        res = __list_foreach(l, visit, p, __list_next);
        break;
    case LIST_WALK_DIR_REV:
        res = __list_foreach(l, visit, p, __list_prev);
        break;
    }

    return res;
}

struct list_find_priv
{
    cstl_compare_func_t * cmp;
    void * p;
    const void * e;
};

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
    int res;

    lfp.cmp = cmp;
    lfp.p   = cmp_p;
    lfp.e   = e;

    res = list_foreach((struct list *)l, list_find_visit, &lfp, dir);
    if (res <= 0) {
        lfp.e = NULL;
    }

    return (void *)lfp.e;
}

static void __list_move(struct list_node * const nh,
                        struct list_node * const oh)
{
    if (oh->n == oh->p && oh->n == oh) {
        nh->n = nh->p = nh;
    } else {
        *nh = *oh;

        nh->n->p = nh;
        nh->p->n = nh;

        oh->n = oh->p = oh;
    }
}

void list_swap(struct list * const a, struct list * const b)
{
    struct list_node t;
    size_t tsz;

    __list_move(&t, &a->h);
    __list_move(&a->h, &b->h);
    __list_move(&b->h, &t);

    cstl_swap(&a->size, &b->size, &tsz, sizeof(tsz));
    cstl_swap(&a->off, &b->off, &tsz, sizeof(tsz));
}

struct list_clear_priv
{
    struct list * l;
    cstl_clear_func_t * clr;
};

static int list_clear_visit(void * const e, void * const p)
{
    struct list_clear_priv * const lcp = p;
    list_erase(lcp->l, e);
    lcp->clr(e, NULL);
    return 0;
}

void list_clear(struct list * const l, cstl_clear_func_t * const clr)
{
    struct list_clear_priv lcp;

    lcp.l = l;
    lcp.clr = clr;

    list_foreach(l, list_clear_visit, &lcp, LIST_WALK_DIR_FWD);
}

void list_reverse(struct list * const l)
{
    struct list_node * c, * p;

    for (c = l->h.p, p = &l->h; c != p; p = c, c = l->h.p) {
        __list_erase(l, c);
        __list_insert(l, p, c);
    }
}

void list_concat(struct list * const d, struct list * const s)
{
    if (d != s && d->off == s->off && s->size > 0) {
        s->h.n->p = d->h.p;
        s->h.p->n = &d->h;

        d->h.p->n = s->h.n;
        d->h.p = s->h.p;

        d->size += s->size;

        list_init(s, s->off);
    }
}

void list_sort(struct list * const l,
               cstl_compare_func_t * const cmp, void * const cmp_p)
{
    if (l->size > 1) {
        struct list _l[2];
        struct list_node * t;

        list_init(&_l[0], l->off);
        list_init(&_l[1], l->off);

        list_concat(&_l[1], l);

        for (t = &_l[1].h;
             _l[0].size < _l[1].size / 2;
             t = t->n, _l[0].size++)
            ;

        _l[0].h.n = _l[1].h.n;
        _l[0].h.p = t;
        _l[1].h.n = t->n;

        _l[0].h.n->p = &_l[0].h;
        _l[0].h.p->n = &_l[0].h;
        _l[1].h.n->p = &_l[1].h;

        _l[1].size -= _l[0].size;

        list_sort(&_l[0], cmp, cmp_p);
        list_sort(&_l[1], cmp, cmp_p);

        while (_l[0].size > 0 && _l[1].size > 0) {
            struct list_node * n;
            struct list * ol;

            if (cmp(__list_element(&_l[0], _l[0].h.n),
                    __list_element(&_l[1], _l[1].h.n),
                    cmp_p) <= 0) {
                ol = &_l[0];
            } else {
                ol = &_l[1];
            }

            n = ol->h.n;
            __list_erase(ol, n);
            __list_insert(l, l->h.p, n);
        }

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
    struct list l;

    LIST_INIT(&l, struct integer, ln);

    __test__list_fill(&l, n);

    list_clear(&l, __test_list_free);
    ck_assert_uint_eq(list_size(&l), 0);
}
END_TEST

START_TEST(concat)
{
    static const size_t n = 4;
    struct list l1, l2;

    LIST_INIT(&l1, struct integer, ln);
    LIST_INIT(&l2, struct integer, ln);

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
    struct list l;

    struct integer * in = NULL;

    LIST_INIT(&l, struct integer, ln);

    __test__list_fill(&l, n);

    list_sort(&l, cmp_integer, NULL);
    ck_assert_uint_eq(n, list_size(&l));
    list_foreach(&l, list_verify_sorted, &in, LIST_WALK_DIR_FWD);

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
    struct list l;

    struct integer * in = NULL;

    LIST_INIT(&l, struct integer, ln);

    __test__list_fill(&l, n);

    list_sort(&l, cmp_integer, NULL);
    list_reverse(&l);
    list_foreach(&l, list_verify_sorted_rev, &in, LIST_WALK_DIR_FWD);

    list_clear(&l, __test_list_free);
    ck_assert_uint_eq(list_size(&l), 0);
}
END_TEST

START_TEST(swap)
{
    struct list l1, l2;

    LIST_INIT(&l1, struct integer, ln);
    LIST_INIT(&l2, struct integer, ln);

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
