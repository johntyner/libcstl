#include "memory.h"

void unique_ptr_alloc(struct unique_ptr * const up, const size_t sz)
{
    unique_ptr_reset(up);
    if (sz > 0) {
        up->ptr = cstl_malloc(sz);
    }
}

cstl_memory_free_t * unique_ptr_get_free(struct unique_ptr * const up)
{
    (void)up;
    return cstl_free;
}

void unique_ptr_reset(struct unique_ptr * const up)
{
    cstl_free(up->ptr);
    up->ptr = NULL;
}

void shared_ptr_alloc(struct shared_ptr * const sp, const size_t sz)
{
    if (sz > 0) {
        sp->data = cstl_malloc(sizeof(*sp->data));
        if (sp->data != NULL) {
            atomic_init(&sp->data->ref.hard, 1);
            atomic_init(&sp->data->ref.soft, 1);

            unique_ptr_init(&sp->data->up);
            unique_ptr_alloc(&sp->data->up, sz);

            if (unique_ptr_get(&sp->data->up) == NULL) {
                cstl_free(sp->data);
                sp->data = NULL;
            }
        }
    }
}

void * shared_ptr_get(struct shared_ptr * const sp)
{
    if (sp->data != NULL) {
        return unique_ptr_get(&sp->data->up);
    }
    return NULL;
}

void shared_ptr_share(struct shared_ptr * const e, struct shared_ptr * const n)
{
    shared_ptr_reset(n);
    if (e->data != NULL) {
        atomic_fetch_add(&e->data->ref.hard, 1);
        atomic_fetch_add(&e->data->ref.soft, 1);

        n->data = e->data;
    }
}

void shared_ptr_reset(struct shared_ptr * const sp)
{
    if (sp->data != NULL) {
        if (atomic_fetch_sub(&sp->data->ref.hard, 1) == 1) {
            unique_ptr_reset(&sp->data->up);
        }

        if (atomic_fetch_sub(&sp->data->ref.soft, 1) == 1) {
            cstl_free(sp->data);
        }

        sp->data = NULL;
    }
}

void weak_ptr_from(struct weak_ptr * const wp, struct shared_ptr * const sp)
{
    weak_ptr_reset(wp);
    if (sp->data != NULL) {
        atomic_fetch_add(&sp->data->ref.soft, 1);

        wp->data = sp->data;
    }
}

void weak_ptr_hold(struct weak_ptr * const wp, struct shared_ptr * const sp)
{
    shared_ptr_reset(sp);
    if (wp->data != NULL) {
        if (atomic_fetch_add(&wp->data->ref.hard, 1) > 0
            && unique_ptr_get(&wp->data->up) != NULL) {
            atomic_fetch_add(&wp->data->ref.soft, 1);
            sp->data = wp->data;
        } else {
            atomic_fetch_sub(&wp->data->ref.hard, 1);
        }
    }
}

void weak_ptr_reset(struct weak_ptr * const wp)
{
    if (wp->data != NULL) {
        if (atomic_fetch_sub(&wp->data->ref.soft, 1) == 1) {
            cstl_free(wp->data);
        }

        wp->data = NULL;
    }
}

#ifdef __cfg_test__
#include <check.h>

START_TEST(unique)
{
    struct unique_ptr p;

    unique_ptr_init(&p);

    unique_ptr_alloc(&p, 512);
    ck_assert_ptr_ne(unique_ptr_get(&p), NULL);

    unique_ptr_reset(&p);
    ck_assert_ptr_eq(unique_ptr_get(&p), NULL);

    unique_ptr_alloc(&p, 1024);
    ck_assert_ptr_ne(unique_ptr_get(&p), NULL);

    unique_ptr_get_free(&p)(unique_ptr_get(&p));
    ck_assert_ptr_ne(unique_ptr_get(&p), NULL);

    unique_ptr_release(&p);
    ck_assert_ptr_eq(unique_ptr_get(&p), NULL);

    unique_ptr_reset(&p);
    ck_assert_ptr_eq(unique_ptr_get(&p), NULL);
}
END_TEST

START_TEST(shared)
{
    struct shared_ptr sp1, sp2;

    shared_ptr_init(&sp1);
    shared_ptr_init(&sp2);

    shared_ptr_alloc(&sp1, 128);
    memset(shared_ptr_get(&sp1), 0, 128);

    shared_ptr_share(&sp1, &sp2);
    ck_assert_ptr_eq(shared_ptr_get(&sp1), shared_ptr_get(&sp2));

    shared_ptr_reset(&sp1);
    ck_assert_ptr_eq(shared_ptr_get(&sp1), NULL);
    ck_assert_ptr_ne(shared_ptr_get(&sp1), shared_ptr_get(&sp2));

    memset(shared_ptr_get(&sp2), 0, 128);
    shared_ptr_reset(&sp2);
}
END_TEST

START_TEST(weak)
{
    struct shared_ptr sp1, sp2;
    struct weak_ptr wp;

    shared_ptr_init(&sp1);
    shared_ptr_init(&sp2);
    weak_ptr_init(&wp);

    shared_ptr_alloc(&sp1, 128);
    memset(shared_ptr_get(&sp1), 0, 128);

    weak_ptr_from(&wp, &sp1);
    shared_ptr_share(&sp1, &sp2);
    ck_assert_ptr_eq(shared_ptr_get(&sp1), shared_ptr_get(&sp2));

    shared_ptr_reset(&sp1);
    ck_assert_ptr_eq(shared_ptr_get(&sp1), NULL);
    ck_assert_ptr_ne(shared_ptr_get(&sp1), shared_ptr_get(&sp2));
    memset(shared_ptr_get(&sp2), 0, 128);

    weak_ptr_hold(&wp, &sp1);
    ck_assert_ptr_eq(shared_ptr_get(&sp1), shared_ptr_get(&sp2));

    shared_ptr_reset(&sp2);
    shared_ptr_reset(&sp1);

    weak_ptr_hold(&wp, &sp1);
    ck_assert_ptr_eq(shared_ptr_get(&sp1), NULL);

    weak_ptr_reset(&wp);
}
END_TEST

Suite * memory_suite(void)
{
    Suite * const s = suite_create("memory");

    TCase * tc;

    tc = tcase_create("memory");
    tcase_add_test(tc, unique);
    tcase_add_test(tc, shared);
    tcase_add_test(tc, weak);
    suite_add_tcase(s, tc);

    return s;
}

#endif
