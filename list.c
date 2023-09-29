#include "list.h"

#include <stdint.h>

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

static int __list_walk(struct list * const l,
                       int (* const visit)(void *, void *), void * const p,
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


int list_walk(struct list * const l,
              int (* const visit)(void *, void *), void * const p,
              const list_walk_dir_t dir)
{
    int res = 0;

    switch (dir) {
    case LIST_WALK_DIR_FWD:
        res = __list_walk(l, visit, p, __list_next);
        break;
    case LIST_WALK_DIR_REV:
        res = __list_walk(l, visit, p, __list_prev);
        break;
    }

    return res;
}

struct list_find_priv
{
    int (* cmp)(const void *, const void *);
    const void * e;
};

static int list_find_visit(void * const e, void * const p)
{
    struct list_find_priv * lfp = p;

    if (lfp->cmp(lfp->e, e) == 0) {
        lfp->e = e;
        return 1;
    }

    return 0;
}

void * list_find(const struct list * const l, const void * const e,
                 int (* const cmp)(const void *, const void *),
                 const list_walk_dir_t dir)
{
    struct list_find_priv lfp;
    int res;

    lfp.cmp = cmp;
    lfp.e   = e;

    res = list_walk((struct list *)l, list_find_visit, &lfp, dir);
    if (res <= 0) {
        lfp.e = NULL;
    }

    return (void *)lfp.e;
}

struct list_clear_priv
{
    struct list * l;
    void (* clr)(void *);
};

static int list_clear_visit(void * const e, void * const p)
{
    struct list_clear_priv * const lcp = p;
    list_erase(lcp->l, e);
    lcp->clr(e);
    return 0;
}

void list_clear(struct list * const l, void (* const clr)(void *))
{
    struct list_clear_priv lcp;

    lcp.l = l;
    lcp.clr = clr;

    list_walk(l, list_clear_visit, &lcp, LIST_WALK_DIR_FWD);
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
               int (* const cmp)(const void *, const void *))
{
    if (l->size > 1) {
        struct list _l[2];

        list_init(&_l[0], l->off);
        list_init(&_l[1], l->off);

        while (_l[0].size < l->size) {
            struct list_node * const c = l->h.n;
            __list_erase(l, c);
            __list_insert(&_l[0], &_l[0].h, c);
        }

        list_concat(&_l[1], l);

        list_sort(&_l[0], cmp);
        list_sort(&_l[1], cmp);

        while (_l[0].size > 0 && _l[1].size > 0) {
            struct list_node * n;
            struct list * ol;

            if (cmp(__list_element(&_l[0], _l[0].h.n),
                    __list_element(&_l[1], _l[1].h.n)) <= 0) {
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

static int cmp_integer(const void * const a, const void * const b)
{
    return ((struct integer *)a)->v - ((struct integer *)b)->v;
}

void __test__list_fill(struct list * l, const size_t n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        struct integer * in = malloc(sizeof(*in));

        in->v = rand() % n;
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

    list_clear(&l, free);
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

    list_clear(&l1, free);
    list_clear(&l2, free);
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

    list_sort(&l, cmp_integer);
    list_walk(&l, list_verify_sorted, &in, LIST_WALK_DIR_FWD);

    list_clear(&l, free);
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

    list_sort(&l, cmp_integer);
    list_reverse(&l);
    list_walk(&l, list_verify_sorted_rev, &in, LIST_WALK_DIR_FWD);

    list_clear(&l, free);
    ck_assert_uint_eq(list_size(&l), 0);
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
    suite_add_tcase(s, tc);

    return s;
}

#endif
