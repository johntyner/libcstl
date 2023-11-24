#include "internal/bench.h"
#include "cstl/vector.h"
#include <stdlib.h>

static int cmp_int(const void * const a, const void * const b, void * const p)
{
    (void)p;
    return *(int *)a - *(int *)b;
}

static void bench_sort(struct bench_context * const ctx,
                       const unsigned long count,
                       const cstl_sort_algorithm_t algo)
{
    const unsigned int n = 3271;

    DECLARE_CSTL_VECTOR(v, int);
    unsigned int i;

    bench_stop_timer(ctx);

    cstl_vector_resize(&v, n);

    for (i = 0; i < count; i++) {
        unsigned int j;

        for (j = 0; j < n; j++) {
            *(int *)cstl_vector_at(&v, j) = rand() % n;
        }

        bench_start_timer(ctx);
        __cstl_vector_sort(&v, cmp_int, NULL, cstl_swap, algo);
        bench_stop_timer(ctx);
    }

    cstl_vector_clear(&v);

    bench_start_timer(ctx);
}

void bench_qsort(struct bench_context * const ctx, const unsigned long count)
{
    bench_sort(ctx, count, CSTL_SORT_ALGORITHM_QUICK);
}

void bench_qsort_r(struct bench_context * const ctx, const unsigned long count)
{
    bench_sort(ctx, count, CSTL_SORT_ALGORITHM_QUICK_R);
}

void bench_qsort_m(struct bench_context * const ctx, const unsigned long count)
{
    bench_sort(ctx, count, CSTL_SORT_ALGORITHM_QUICK_M);
}

void bench_hsort(struct bench_context * const ctx, const unsigned long count)
{
    bench_sort(ctx, count, CSTL_SORT_ALGORITHM_HEAP);
}
