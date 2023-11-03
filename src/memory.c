/*!
 * @file
 */

#include "cstl/memory.h"

#include <assert.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sched.h>

/*
 * cstl_shared_ptr_data is used to manage memory allocated by shared pointers.
 * it is also pointed to by the weak pointer objects.
 */
struct cstl_shared_ptr_data
{
    struct {
        /*
         * hard refers to the number of shared pointers that manage
         * the memory. soft is the number of shared and weak pointers
         * that point to this cstl_shared_ptr_data structure.
         *
         * the lock member is used to prevent weak pointers from racing
         * with each other when trying to convert to a shared pointer
         */
        atomic_size_t hard, soft;
        atomic_flag lock;
    } ref;
    /*
     * The unique pointer is a pointer to the managed memory.
     * There may be many shared and weak pointers pointing to
     * this object, but this object is the only thing
     * containing/pointing to the memory managed within the
     * unique pointer.
     */
    cstl_unique_ptr_t up;
};

void cstl_unique_ptr_alloc(cstl_unique_ptr_t * const up, const size_t sz,
                           cstl_xtor_func_t * const clr, void * const priv)
{
    cstl_unique_ptr_reset(up);
    if (sz > 0) {
        void * const ptr = malloc(sz);
        if (ptr != NULL) {
            cstl_guarded_ptr_set(&up->gp, ptr);
            up->clr.func = clr;
            up->clr.priv = priv;
        }
    }
}

void cstl_unique_ptr_reset(cstl_unique_ptr_t * const up)
{
    void * const ptr = cstl_guarded_ptr_get(&up->gp);
    if (up->clr.func != NULL) {
        up->clr.func(ptr, up->clr.priv);
    }
    free(ptr);
    cstl_unique_ptr_init(up);
}

void cstl_shared_ptr_alloc(cstl_shared_ptr_t * const sp, const size_t sz,
                           cstl_xtor_func_t * const clr)
{
    cstl_shared_ptr_reset(sp);

    if (sz > 0) {
        struct cstl_shared_ptr_data * const data = malloc(sizeof(*data));

        cstl_guarded_ptr_set(&sp->data, data);
        if (data != NULL) {
            atomic_init(&data->ref.hard, 1);
            atomic_init(&data->ref.soft, 1);
            atomic_flag_clear(&data->ref.lock);

            cstl_unique_ptr_init(&data->up);
            cstl_unique_ptr_alloc(&data->up, sz, clr, NULL);

            if (cstl_unique_ptr_get(&data->up) == NULL) {
                cstl_guarded_ptr_set(&sp->data, NULL);
                free(data);
            }
        }
    }
}

bool cstl_shared_ptr_unique(const cstl_shared_ptr_t * const sp)
{
    const struct cstl_shared_ptr_data * const data =
        cstl_guarded_ptr_get_const(&sp->data);
    int count = 1;
    if (data != NULL) {
        count = atomic_load(&data->ref.soft);
    }
    return count == 1;
}

const void * cstl_shared_ptr_get_const(const cstl_shared_ptr_t * const sp)
{
    const struct cstl_shared_ptr_data * const data =
        cstl_guarded_ptr_get_const(&sp->data);
    if (data != NULL) {
        return cstl_unique_ptr_get_const(&data->up);
    }
    return NULL;
}

void cstl_shared_ptr_share(const cstl_shared_ptr_t * const e,
                           cstl_shared_ptr_t * const n)
{
    struct cstl_shared_ptr_data * data;

    cstl_shared_ptr_reset(n);
    cstl_guarded_ptr_copy(&n->data, &e->data);

    data = cstl_guarded_ptr_get(&n->data);
    if (data != NULL) {
        atomic_fetch_add(&data->ref.hard, 1);
        atomic_fetch_add(&data->ref.soft, 1);
    }
}

void cstl_shared_ptr_reset(cstl_shared_ptr_t * const sp)
{
    struct cstl_shared_ptr_data * const data =
        cstl_guarded_ptr_get(&sp->data);

    if (data != NULL) {
        if (atomic_fetch_sub(&data->ref.hard, 1) == 1) {
            cstl_unique_ptr_reset(&data->up);
        }

        /*
         * manage the shared data structure via the
         * weak pointer code; it's the same handling
         */
        cstl_weak_ptr_reset(sp);
    }
}

void cstl_weak_ptr_from(cstl_weak_ptr_t * const wp,
                        const cstl_shared_ptr_t * const sp)
{
    struct cstl_shared_ptr_data * data;

    cstl_weak_ptr_reset(wp);
    cstl_guarded_ptr_copy(&wp->data, &sp->data);

    data = cstl_guarded_ptr_get(&wp->data);
    if (data != NULL) {
        atomic_fetch_add(&data->ref.soft, 1);
    }
}

void cstl_weak_ptr_lock(const cstl_weak_ptr_t * const wp,
                        cstl_shared_ptr_t * const sp)
{
    struct cstl_shared_ptr_data * data;

    cstl_shared_ptr_reset(sp);
    cstl_guarded_ptr_copy(&sp->data, &wp->data);

    data = cstl_guarded_ptr_get(&sp->data);
    if (data != NULL) {
        /*
         * the weak pointer wants to increment the hard reference
         * only if the hard reference is already greater than 0.
         * the atomic interfaces doesn't allowing checking the current
         * count until *after* the reference is incremented. this
         * means that another weak pointer could race with this one
         * and see a value of 1 when it does its check and assume
         * that the memory is live.
         *
         * in order to prevent this race, the code "spins" on this
         * lock flag because the operations done under the lock are
         * non-blocking, and the code currently holding the lock
         * should exit quickly.
         */
        while (atomic_flag_test_and_set(&data->ref.lock)) {
            sched_yield();
        }

        /*
         * since we can't race with other weak pointers, if the
         * counter is greater than 0, we know that the underlying
         * memory is live. it is not possible for this code to
         * race with a shared ptr--in the sense that a shared pointer
         * erroneously sees a 1 in the hard counter and assumes
         * that the underlying memory is live--because if the hard
         * counter was 0, that means that there was no live shared
         * pointer from which to try to share
         */

        if (atomic_fetch_add(&data->ref.hard, 1) > 0) {
            /*
             * the memory is live, add a reference
             * to the shared data structure too
             */
            atomic_fetch_add(&data->ref.soft, 1);
        } else {
            /* the memory wasn't live, put the counter back */
            atomic_fetch_sub(&data->ref.hard, 1);
            cstl_guarded_ptr_set(&sp->data, NULL);
        }

        atomic_flag_clear(&data->ref.lock);
    }
}

void cstl_weak_ptr_reset(cstl_weak_ptr_t * const wp)
{
    struct cstl_shared_ptr_data * const data =
        cstl_guarded_ptr_get(&wp->data);

    if (data != NULL) {
        cstl_guarded_ptr_set(&wp->data, NULL);

        if (atomic_fetch_sub(&data->ref.soft, 1) == 1) {
            free(data);
        }
    }
}

#ifdef __cfg_test__
// GCOV_EXCL_START
#include <check.h>

START_TEST(unique)
{
    DECLARE_CSTL_UNIQUE_PTR(p);
    cstl_xtor_func_t * dtor;
    void * priv;

    cstl_unique_ptr_alloc(&p, 512, NULL, NULL);
    ck_assert_ptr_nonnull(cstl_unique_ptr_get(&p));

    cstl_unique_ptr_reset(&p);
    ck_assert_ptr_null(cstl_unique_ptr_get(&p));

    cstl_unique_ptr_alloc(&p, 1024, NULL, NULL);
    ck_assert_ptr_nonnull(cstl_unique_ptr_get(&p));

    free(cstl_unique_ptr_get(&p));
    ck_assert_ptr_nonnull(cstl_unique_ptr_get(&p));

    cstl_unique_ptr_release(&p, &dtor, &priv);
    ck_assert_ptr_eq(cstl_unique_ptr_get(&p), NULL);
    ck_assert_uint_eq((uintptr_t)dtor, (uintptr_t)NULL);
    ck_assert_ptr_null(priv);

    cstl_unique_ptr_reset(&p);
    ck_assert_ptr_null(cstl_unique_ptr_get(&p));
}
END_TEST

START_TEST(shared)
{
    DECLARE_CSTL_SHARED_PTR(sp1);
    DECLARE_CSTL_SHARED_PTR(sp2);

    cstl_shared_ptr_alloc(&sp1, 128, NULL);
    memset(cstl_shared_ptr_get(&sp1), 0, 128);

    cstl_shared_ptr_share(&sp1, &sp2);
    ck_assert_ptr_eq(cstl_shared_ptr_get(&sp1), cstl_shared_ptr_get(&sp2));

    cstl_shared_ptr_reset(&sp1);
    ck_assert_ptr_eq(cstl_shared_ptr_get(&sp1), NULL);
    ck_assert_ptr_ne(cstl_shared_ptr_get(&sp1), cstl_shared_ptr_get(&sp2));

    memset(cstl_shared_ptr_get(&sp2), 0, 128);
    cstl_shared_ptr_reset(&sp2);
}
END_TEST

START_TEST(weak)
{
    DECLARE_CSTL_SHARED_PTR(sp1);
    DECLARE_CSTL_SHARED_PTR(sp2);
    DECLARE_CSTL_WEAK_PTR(wp);

    cstl_shared_ptr_alloc(&sp1, 128, NULL);
    memset(cstl_shared_ptr_get(&sp1), 0, 128);

    cstl_weak_ptr_from(&wp, &sp1);
    cstl_shared_ptr_share(&sp1, &sp2);
    ck_assert_ptr_eq(cstl_shared_ptr_get(&sp1), cstl_shared_ptr_get(&sp2));

    cstl_shared_ptr_reset(&sp1);
    ck_assert_ptr_eq(cstl_shared_ptr_get(&sp1), NULL);
    ck_assert_ptr_ne(cstl_shared_ptr_get(&sp1), cstl_shared_ptr_get(&sp2));
    memset(cstl_shared_ptr_get(&sp2), 0, 128);

    cstl_weak_ptr_lock(&wp, &sp1);
    ck_assert_ptr_eq(cstl_shared_ptr_get(&sp1), cstl_shared_ptr_get(&sp2));

    cstl_shared_ptr_reset(&sp2);
    cstl_shared_ptr_reset(&sp1);

    cstl_weak_ptr_lock(&wp, &sp1);
    ck_assert_ptr_eq(cstl_shared_ptr_get(&sp1), NULL);

    cstl_weak_ptr_reset(&wp);
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

// GCOV_EXCL_STOP
#endif
