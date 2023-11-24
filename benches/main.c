#include "internal/bench.h"

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define CLOCK_BENCH     CLOCK_PROCESS_CPUTIME_ID
#define NS_PER_S        1000000000

struct stdev_context
{
    unsigned int n;
    float M, S;
};

struct bench_context
{
    struct stdev_context stdev;

    struct timespec start, accum;
};

static inline void timespec_set(struct timespec * const ts,
                                const time_t sec, const long nsec)
{
    ts->tv_sec = sec;
    ts->tv_nsec = nsec;
}

static inline void timespec_clr(struct timespec * const ts)
{
    timespec_set(ts, 0, 0);
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

static void stdev_init(struct stdev_context * const ctx)
{
    ctx->n = 0;
    ctx->M = 0;
    ctx->S = 0;
}

static void stdev_update(struct stdev_context * const ctx, const float s)
{
    const float sM = s - ctx->M;

    ctx->n++;
    ctx->M += sM / ctx->n;

    ctx->S += sM * (s - ctx->M);
}

static unsigned int stdev_samples(const struct stdev_context * const ctx)
{
    return ctx->n;
}

static float stdev_mean(const struct stdev_context * const ctx)
{
    return ctx->M;
}

static float stdev_final(const struct stdev_context * const ctx)
{
    return sqrtf(ctx->S / ctx->n);
}

static void bench_context_init(struct bench_context * const ctx)
{
    stdev_init(&ctx->stdev);

    timespec_clr(&ctx->accum);
    timespec_set(&ctx->start, -1, 0);
}

void bench_stop_timer(struct bench_context * const ctx)
{
    if (ctx->start.tv_sec >= 0) {
        struct timespec dur;

        clock_gettime(CLOCK_BENCH, &dur);

        timespec_sub(&dur, &ctx->start, &dur);
        timespec_add(&ctx->accum, &dur, &ctx->accum);

        ctx->start.tv_sec = -1;
    }
}

void bench_start_timer(struct bench_context * const ctx)
{
    if (ctx->start.tv_sec < 0) {
        clock_gettime(CLOCK_BENCH, &ctx->start);
    }
}

static void __bench_run(struct bench_context * const ctx,
                        bench_runner_func_t * const run,
                        const unsigned long count)
{
    float tm;

    timespec_clr(&ctx->accum);
    timespec_set(&ctx->start, -1, 0);

    bench_start_timer(ctx);
    run(ctx, count);
    bench_stop_timer(ctx);

    tm = timespec_as_float(&ctx->accum) / count;
    for (unsigned int i = 0; i < count; i++) {
        stdev_update(&ctx->stdev, tm);
    }
}

static void bench_run(const char * const name, bench_runner_func_t * const run)
{
    struct bench_context ctx;
    struct timespec accum;
    unsigned int runs;
    int factor = -1;
    float cv;

    printf("running %-20s", name); fflush(stdout);

    bench_context_init(&ctx);
    do {
        factor++;
        runs = 1 << factor;
        __bench_run(&ctx, run, runs);
    } while (ctx.accum.tv_sec < 1
             && ctx.accum.tv_nsec < 100000);

    bench_context_init(&ctx);
    timespec_clr(&accum);

    for (unsigned int i = 0; i < 100; i++) {
        __bench_run(&ctx, run, runs);
        timespec_add(&accum, &ctx.accum, &accum);
    }

    do {
        __bench_run(&ctx, run, runs);
        cv = stdev_final(&ctx.stdev) / stdev_mean(&ctx.stdev);
        timespec_add(&accum, &ctx.accum, &accum);
    } while (cv > 0.05f && accum.tv_sec < 3);

    printf(" %8u %14.9f sec/iter (+/- %.2f%%)\n",
           stdev_samples(&ctx.stdev), stdev_mean(&ctx.stdev),
           100 * (stdev_final(&ctx.stdev) / stdev_mean(&ctx.stdev)));
}

int main(void)
{
#define BENCH_RUN(FUNC)                         \
    do {                                        \
        extern bench_runner_func_t FUNC;        \
        bench_run(#FUNC, FUNC);                 \
    } while (0)

    /* warm up the cpu */
    for (volatile int32_t x = 0; x >= 0; x++)
        ;

    BENCH_RUN(bench_qsort);
    BENCH_RUN(bench_qsort_r);
    BENCH_RUN(bench_qsort_m);
    BENCH_RUN(bench_hsort);

    BENCH_RUN(bench_map_insert);

    return 0;
}
