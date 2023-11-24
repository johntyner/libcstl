#include "internal/bench.h"

#include <stdio.h>

void bench_stop_timer(struct bench_context * const ctx)
{
    if (ctx->start.tv_sec >= 0) {
        struct timespec dur;

        clock_gettime(CLOCK_BENCH, &dur);

        dur.tv_nsec -= ctx->start.tv_nsec;
        if (dur.tv_nsec < 0) {
            dur.tv_nsec += 1000000000;
            dur.tv_sec--;
        }
        dur.tv_sec -= ctx->start.tv_sec;

        ctx->accum.tv_nsec += dur.tv_nsec;
        if (ctx->accum.tv_nsec >= 1000000000) {
            ctx->accum.tv_nsec -= 1000000000;
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
    DECLARE_BENCH_CONTEXT(ctx);
    float dur;

    printf("running %-20s", name);
    fflush(stdout);

    ctx.count = 1;
    bench_start_timer(&ctx);
    run(&ctx);
    bench_stop_timer(&ctx);

    dur = (float)ctx.accum.tv_nsec / 1000000000;
    dur += ctx.accum.tv_sec;

    printf("%8lu%20.9f s/iter\n", ctx.count, dur / ctx.count);
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

    return 0;
}
