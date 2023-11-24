#include "internal/bench.h"

#include <stdio.h>

#define NS_PER_S        1000000000

void bench_stop_timer(struct bench_context * const ctx)
{
    if (ctx->start.tv_sec >= 0) {
        struct timespec dur;

        clock_gettime(CLOCK_BENCH, &dur);

        dur.tv_nsec -= ctx->start.tv_nsec;
        if (dur.tv_nsec < 0) {
            dur.tv_nsec += NS_PER_S;
            dur.tv_sec--;
        }
        dur.tv_sec -= ctx->start.tv_sec;

        ctx->accum.tv_nsec += dur.tv_nsec;
        if (ctx->accum.tv_nsec >= NS_PER_S) {
            ctx->accum.tv_nsec -= NS_PER_S;
            ctx->accum.tv_sec++;
        }
        ctx->accum.tv_sec += dur.tv_sec;

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
    DECLARE_BENCH_CONTEXT(_ctx);
    DECLARE_BENCH_CONTEXT(ctx);
    float dur;

    printf("running %-20s", name); fflush(stdout);

    _ctx.count = 100;
    bench_start_timer(&_ctx);
    run(&_ctx);
    bench_stop_timer(&_ctx);

    ctx.count = 100;
    bench_start_timer(&ctx);
    run(&ctx);
    bench_stop_timer(&ctx);

    dur = (float)ctx.accum.tv_nsec / NS_PER_S;
    dur += ctx.accum.tv_sec;

    printf("%8lu%20.9f sec/iter\n", ctx.count, dur / ctx.count);
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
