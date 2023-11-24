#include "internal/bench.h"

#include <stdio.h>
#include <time.h>

#define CLOCK_BENCH     CLOCK_PROCESS_CPUTIME_ID
#define NS_PER_S        1000000000

struct bench_context
{
    unsigned long count;

    struct timespec start;
    struct timespec accum;
};

static inline void timespec_clr(struct timespec * const ts)
{
    ts->tv_sec = 0;
    ts->tv_nsec = 0;
}

static inline void timespec_add(
    const struct timespec * const a, const struct timespec * const b,
    struct timespec * const res)
{
    res->tv_sec = a->tv_sec + b->tv_sec;
    res->tv_nsec = a->tv_nsec + b->tv_nsec;
    if (res->tv_nsec >= NS_PER_S) {
        res->tv_nsec -= NS_PER_S;
        res->tv_sec++;
    }
}

static inline void timespec_sub(
    const struct timespec * const a, const struct timespec * const b,
    struct timespec * const res)
{
    res->tv_sec = a->tv_sec - b->tv_sec;
    res->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (res->tv_nsec < 0) {
        res->tv_nsec += NS_PER_S;
        res->tv_sec--;
    }
}

static inline float timespec_as_float(const struct timespec * const ts)
{
    return (float)ts->tv_nsec / NS_PER_S + ts->tv_sec;
}

static void bench_context_init(struct bench_context * const ctx)
{
    ctx->count = 0;
    timespec_clr(&ctx->accum);
    timespec_clr(&ctx->start);
    ctx->start.tv_sec = -1;
}

unsigned long bench_context_count(const struct bench_context * const ctx)
{
    return ctx->count;
}

void bench_stop_timer(struct bench_context * const ctx)
{
    if (ctx->start.tv_sec >= 0) {
        struct timespec dur;

        clock_gettime(CLOCK_BENCH, &dur);

        timespec_sub(&dur, &ctx->start, &dur);
        timespec_add(&dur, &ctx->accum, &ctx->accum);

        ctx->start.tv_sec = -1;
    }
}

void bench_start_timer(struct bench_context * const ctx)
{
    if (ctx->start.tv_sec < 0) {
        clock_gettime(CLOCK_BENCH, &ctx->start);
    }
}

static void bench_run(const char * const name, bench_runner_func_t * const run)
{
    struct bench_context ctx;

    printf("running %-20s", name); fflush(stdout);

    bench_context_init(&ctx);
    ctx.count = 100;
    bench_start_timer(&ctx);
    run(&ctx);
    bench_stop_timer(&ctx);

    bench_context_init(&ctx);
    ctx.count = 100;
    bench_start_timer(&ctx);
    run(&ctx);
    bench_stop_timer(&ctx);

    printf("%8lu%20.9f sec/iter\n",
           ctx.count,
           timespec_as_float(&ctx.accum) / ctx.count);
}

int main(void)
{
#define BENCH_RUN(FUNC)                         \
    do {                                        \
        extern bench_runner_func_t FUNC;        \
        bench_run(#FUNC, FUNC);                 \
    } while (0)

    BENCH_RUN(bench_qsort);
    BENCH_RUN(bench_qsort_r);
    BENCH_RUN(bench_qsort_m);
    BENCH_RUN(bench_hsort);

    BENCH_RUN(bench_map_insert);

    return 0;
}
