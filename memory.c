/*!
 * @file
 */

#include "memory.h"

#include <assert.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sched.h>

/*
 * shared_ptr_data is used to manage memory allocated by shared pointers.
 * it is also pointed to by the weak pointer objects.
 */
struct shared_ptr_data
{
    struct {
        /*
         * hard refers to the number of shared pointers that manage
         * the memory. soft is the number of shared and weak pointers
         * that point to this shared_ptr_data structure.
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
    unique_ptr_t up;
};

void unique_ptr_alloc(unique_ptr_t * const up, const size_t sz,
                      cstl_clear_func_t * const clr)
{
    unique_ptr_reset(up);
    if (sz > 0) {
        void * const ptr = malloc(sz);
        if (ptr != NULL) {
            guarded_ptr_init_set(&up->gp, ptr);
            up->clr = clr;
        }
    }
}

void unique_ptr_reset(unique_ptr_t * const up)
{
    void * const ptr = guarded_ptr_get(&up->gp);
    if (up->clr != NULL) {
        up->clr(ptr, NULL);
    }
    free(ptr);
    unique_ptr_init(up);
}

void shared_ptr_alloc(shared_ptr_t * const sp, const size_t sz,
                      cstl_clear_func_t * const clr)
{
    shared_ptr_reset(sp);

    if (sz > 0) {
        struct shared_ptr_data * const data = malloc(sizeof(*data));

        guarded_ptr_init_set(&sp->data, data);
        if (data != NULL) {
            atomic_init(&data->ref.hard, 1);
            atomic_init(&data->ref.soft, 1);
            atomic_flag_clear(&data->ref.lock);

            unique_ptr_init(&data->up);
            unique_ptr_alloc(&data->up, sz, clr);

            if (unique_ptr_get(&data->up) == NULL) {
                guarded_ptr_init_set(&sp->data, NULL);
                free(data);
            }
        }
    }
}

int shared_ptr_use_count(const shared_ptr_t * const sp)
{
    struct shared_ptr_data * const data = guarded_ptr_get(&sp->data);
    int count = 0;
    if (data != NULL) {
        count = atomic_load(&data->ref.hard);
    }
    return count;
}

void * shared_ptr_get(const shared_ptr_t * const sp)
{
    struct shared_ptr_data * const data = guarded_ptr_get(&sp->data);
    if (data != NULL) {
        return unique_ptr_get(&data->up);
    }
    return NULL;
}

void shared_ptr_share(shared_ptr_t * const e, shared_ptr_t * const n)
{
    struct shared_ptr_data * const data = guarded_ptr_get(&e->data);

    shared_ptr_reset(n);
    if (data != NULL) {
        atomic_fetch_add(&data->ref.hard, 1);
        atomic_fetch_add(&data->ref.soft, 1);

        guarded_ptr_copy(&n->data, &e->data);
    }
}

void shared_ptr_reset(shared_ptr_t * const sp)
{
    struct shared_ptr_data * const data = guarded_ptr_get(&sp->data);

    if (data != NULL) {
        if (atomic_fetch_sub(&data->ref.hard, 1) == 1) {
            unique_ptr_reset(&data->up);
        }

        /*
         * manage the shared data structure via the
         * weak pointer code; it's the same handling
         */
        weak_ptr_reset(sp);
    }
}

void weak_ptr_from(weak_ptr_t * const wp, const shared_ptr_t * const sp)
{
    struct shared_ptr_data * const data = guarded_ptr_get(&sp->data);

    weak_ptr_reset(wp);
    if (data != NULL) {
        atomic_fetch_add(&data->ref.soft, 1);
        guarded_ptr_copy(&wp->data, &sp->data);
    }
}

void weak_ptr_lock(const weak_ptr_t * const wp, shared_ptr_t * const sp)
{
    struct shared_ptr_data * const data = guarded_ptr_get(&wp->data);

    shared_ptr_reset(sp);
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
            guarded_ptr_copy(&sp->data, &wp->data);
        } else {
            /* the memory wasn't live, put the counter back */
            atomic_fetch_sub(&data->ref.hard, 1);
        }

        atomic_flag_clear(&data->ref.lock);
    }
}

void weak_ptr_reset(weak_ptr_t * const wp)
{
    struct shared_ptr_data * const data = guarded_ptr_get(&wp->data);

    if (data != NULL) {
        guarded_ptr_init_set(&wp->data, NULL);

        if (atomic_fetch_sub(&data->ref.soft, 1) == 1) {
            free(data);
        }
    }
}

#ifdef __cfg_test__
#include <check.h>

START_TEST(unique)
{
    DECLARE_UNIQUE_PTR(p);

    unique_ptr_alloc(&p, 512, NULL);
    ck_assert_ptr_ne(unique_ptr_get(&p), NULL);

    unique_ptr_reset(&p);
    ck_assert_ptr_eq(unique_ptr_get(&p), NULL);

    unique_ptr_alloc(&p, 1024, NULL);
    ck_assert_ptr_ne(unique_ptr_get(&p), NULL);

    free(unique_ptr_get(&p));
    ck_assert_ptr_ne(unique_ptr_get(&p), NULL);

    unique_ptr_release(&p, NULL);
    ck_assert_ptr_eq(unique_ptr_get(&p), NULL);

    unique_ptr_reset(&p);
    ck_assert_ptr_eq(unique_ptr_get(&p), NULL);
}
END_TEST

START_TEST(shared)
{
    DECLARE_SHARED_PTR(sp1);
    DECLARE_SHARED_PTR(sp2);

    shared_ptr_alloc(&sp1, 128, NULL);
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
    DECLARE_SHARED_PTR(sp1);
    DECLARE_SHARED_PTR(sp2);
    DECLARE_WEAK_PTR(wp);

    shared_ptr_alloc(&sp1, 128, NULL);
    memset(shared_ptr_get(&sp1), 0, 128);

    weak_ptr_from(&wp, &sp1);
    shared_ptr_share(&sp1, &sp2);
    ck_assert_ptr_eq(shared_ptr_get(&sp1), shared_ptr_get(&sp2));

    shared_ptr_reset(&sp1);
    ck_assert_ptr_eq(shared_ptr_get(&sp1), NULL);
    ck_assert_ptr_ne(shared_ptr_get(&sp1), shared_ptr_get(&sp2));
    memset(shared_ptr_get(&sp2), 0, 128);

    weak_ptr_lock(&wp, &sp1);
    ck_assert_ptr_eq(shared_ptr_get(&sp1), shared_ptr_get(&sp2));

    shared_ptr_reset(&sp2);
    shared_ptr_reset(&sp1);

    weak_ptr_lock(&wp, &sp1);
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
